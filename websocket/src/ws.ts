//@ts-ignore
global.fetch = require("node-fetch");

import ActionCable from "action-cable-node";
//@ts-ignore
import fetch from "node-fetch";
//@ts-ignore
import { gql, HttpLink, InMemoryCache } from "apollo-boost";
//@ts-ignore
import ApolloClient from "apollo-client";
//@ts-ignore
import { ApolloLink, from, split } from "apollo-link";
//@ts-ignore
import { onError } from "apollo-link-error";
//@ts-ignore
import { createHttpLink } from "apollo-link-http";
//@ts-ignore
import getMAC, { isMAC } from "getmac";

import { getMainDefinition } from "apollo-utilities";
import ActionCableLink from "graphql-ruby-client/subscriptions/ActionCableLink";
import jwtDecode from "jwt-decode";
import { LocalStorage } from "node-localstorage";
import ws from "ws";
//@ts-ignore
import MessageQueue from "./messageQueue";
import StateMachine from "./stateMachine";
const isProd = process.env.IS_PRODUCTION === "1";
const deviceMac = getMAC();
console.log("deviceMac", deviceMac);
const isPreProd = process.env.IS_PRODUCTION === "2";
let backendUrl;
if (isProd) {
  backendUrl = "entabeni-api.herokuapp.com";
} else if (isPreProd) {
  backendUrl = "pre-production-api.herokuapp.com";
} else {
  backendUrl = "entabeni-api-staging.herokuapp.com";
}
const websocketUrl = `wss://${backendUrl}/cable/`;

const envPrintTerminalId =
  process.env.PRINT_TERMINAL_ID || "c9fff07d-5470-44a1-ad96-2c05872078ea";
const password =
  process.env.TERMINAL_PASSWORD || "973595bf280d548eb8455d4f2d131561";
console.log("envPrintTerminalId", envPrintTerminalId);
const frontendUrl =
  `https://${backendUrl}/?frontEndUrl=https://${process.env.FRONTEND_URL}/` ||
  "https://entabeni-api-staging.herokuapp.com/?frontEndUrl=https://pos-demo.entabeni.tech/";
console.log("frontendUrl", frontendUrl);
const apiKey = isProd
  ? "335654d2600faead9936251ea066f4a9"
  : "b04f110e-9ae0-4018-8954-4b59de0663e9";
const localStorage = new LocalStorage("./terminal");

interface PrintJobData {
  newPrintJob: {
    id: string;
    printData: string;
    printJobType: string;
    resolution: string;
    status: string;
    accessRecordId: string;
    printTerminalId: string;
    saleId: string;
  };
}
interface PrintJobVariables {
  printTerminalId: string;
}
const PRINT_JOBS_SUBSCRIPTION = gql`
  subscription onNewPrintJob($printTerminalId: String!) {
    newPrintJob(printTerminalId: $printTerminalId) {
      id
      printData
      printJobType
      resolution
      status
      accessRecordId
      printTerminalId
      saleId
    }
  }
`;

interface ScanJobData {
  newScanJob: {
    id: string;
    status: string;
    accessRecordId: string;
  };
}
interface ScanJobVariables {
  printTerminalId: string;
}
const SCAN_JOBS_SUBSCRIPTION = gql`
  subscription onNewScanJob($printTerminalId: String!) {
    newScanJob(printTerminalId: $printTerminalId) {
      id
      status
      accessRecordId
    }
  }
`;

export const GET_SALE_DETAILS_BY_ID = gql`
  query sale($id: String!) {
    pos {
      sale(id: $id) {
        saleLineItems {
          guest {
            id
            fullName
            email
          }
        }
      }
    }
  }
`;

const SIGN_IN_TERMINAL_MUTATION = gql`
  mutation SignInTerminal(
    $deviceMac: String!
    $password: String!
    $printTerminalId: String!
  ) {
    pos {
      signInTerminal(
        deviceMac: $deviceMac
        password: $password
        printTerminalId: $printTerminalId
      ) {
        success
        authToken
      }
    }
  }
`;

const UPDATE_PRINT_JOB_MUTATION = gql`
  mutation UpdatePrintJobMutation(
    $printJobId: String!
    $status: String!
    $printJobType: String!
    $errorReason: String
    $resolution: String
  ) {
    pos {
      updatePrintJob(
        id: $printJobId
        status: $status
        printJobType: $printJobType
        errorReason: $errorReason
        resolution: $resolution
      ) {
        id
        printData
        printJobType
        errorReason
        status
        accessRecordId
        printTerminalId
        saleId
      }
    }
  }
`;

const UPDATE_SCAN_JOB_MUTATION = gql`
  mutation UpdateScanJobMutation(
    $scanJobId: String!
    $status: String!
    $cardRfid: String
  ) {
    pos {
      updateScanJob(id: $scanJobId, status: $status, cardRfid: $cardRfid) {
        id
        status
        accessRecordId
      }
    }
  }
`;

const UPDATE_ACCESS_RECORD_MUTATION = gql`
  mutation UpdateAccessRecordMutation(
    $accessRecordId: String!
    $cardRfid: String!
  ) {
    pos {
      updateAccessRecord(id: $accessRecordId, cardRfid: $cardRfid) {
        id
      }
    }
  }
`;

class WebSocket {
  state: StateMachine;
  mq: any;
  apolloClient = new ApolloClient({
    cache: new InMemoryCache(),
    link: createHttpLink({
      uri: `/graphql/`,
    }),
  });
  printJobToRedis: any;
  scanJobToRedis: any;
  constructor(mq: MessageQueue, state: StateMachine) {
    this.state = state;
    this.mq = mq;
    this.printJobToRedis = [];
    this.scanJobToRedis = [];
  }

  initClient(token: null | string, baseUrl: string) {
    const authLink = new ApolloLink((operation, forward) => {
      operation.setContext(() => ({
        headers: {
          authorization: token ? token : null,
        },
      }));
      return forward(operation);
    });

    // Create regular NetworkInterface by using apollo-client's API:
    const httpLink = new HttpLink({
      uri: `${baseUrl}/graphql/`, // Your GraphQL endpoint
      fetch,
    });

    // Create WebSocket client
    const cable = websocketUrl
      ? ActionCable.createConsumer(websocketUrl, ws)
      : null;

    const wsLink = new ActionCableLink({
      cable,
    });

    const errorLink = onError(({ graphQLErrors, networkError, operation }) => {
      if (!!graphQLErrors) {
        graphQLErrors.forEach(({ message, path }) =>
          console.log(`[GraphQL Error]: Message: ${message}, Path: ${path}`)
        );
      }

      if (!!networkError) {
        console.log(
          `[Network error ${operation.operationName}]: Message: ${networkError.message}`
        );
      }
    });

    // Extend the network interface with the WebSocket
    const link = split(
      ({ query }) => {
        const { kind, operation } = getMainDefinition(query);
        return kind === "OperationDefinition" && operation === "subscription";
      },
      wsLink,
      httpLink
    );

    const cache = new InMemoryCache({
      dataIdFromObject: (object) => object.id || null,
    });

    // Finally, create your ApolloClient instance with the modified network interface
    this.apolloClient = new ApolloClient({
      link: from([errorLink, authLink, link]),
      cache,
    });
  }

  connect() {
    const token = localStorage.getItem("jwt");
    fetch(frontendUrl, {
      method: "GET",
      headers: {
        "Content-Type": "application/json",
        "X-Key-Inflection": "camel",
      },
    })
      .then(function(u) {
        return u.json();
      })
      .then((res) => {
        const baseUrl = res.baseUrl;
        console.log("connect -> baseUrl", baseUrl);
        if (token) {
          this.subscribe(token, `${baseUrl}/`);
        } else {
          this.loginThenSubscribe(`${baseUrl}/`);
        }
      })
      .catch(function(error) {
        console.log("error connecting to get baseUrl");
        setTimeout(() => {
          //@ts-ignore
          this.connect();
        }, 15000);
      });
  }

  loginThenSubscribe(baseUrl: string) {
    this.initClient(null, baseUrl);
    // address.mac(function(err, addr) {
    this.apolloClient
      .mutate({
        mutation: SIGN_IN_TERMINAL_MUTATION,
        variables: {
          deviceMac,
          printTerminalId: envPrintTerminalId,
          password,
        },
      })
      .then(({ data }) => {
        const token = data.pos.signInTerminal.authToken;
        localStorage.setItem("jwt", token);
        //@ts-ignore
        this.subscribe(token);
      })
      .catch((error) => {});
    // });
  }

  subscribe(token: string, baseUrl: string) {
    this.initClient(token, baseUrl);
    const tokenDecoded = jwtDecode(token);
    const that = this;
    this.apolloClient
      .subscribe<PrintJobData, PrintJobVariables>({
        query: PRINT_JOBS_SUBSCRIPTION,
        variables: {
          printTerminalId: tokenDecoded.printTerminalId,
        },
        //@ts-ignore
        error(err) {
          console.error("err", err);
        },
      })
      .subscribe({
        next(res) {
          console.log("TCL: next -> res", res);
          const printJob = res.data.newPrintJob;
          if (
            (printJob.status === "created" ||
              printJob.resolution === "retry") &&
            printJob.resolution !== "cancel"
          ) {
            if (
              printJob.status === "created" &&
              printJob.resolution === "retry"
            ) {
              //@ts-ignore
              that.state.idle();
            }
            that.pushPrintJobToQueue(printJob);
          }
          if (
            printJob.resolution === "cancel" &&
            printJob.status !== "complete" &&
            printJob.status !== "processing"
          ) {
            that.clearQueue(printJob);
          }
        },
        error(err: string) {
          console.error("err", err);
        },
      });

    this.apolloClient
      .subscribe<ScanJobData, ScanJobVariables>({
        query: SCAN_JOBS_SUBSCRIPTION,
        variables: {
          printTerminalId: tokenDecoded.printTerminalId,
        },
      })
      .subscribe({
        next(res) {
          console.log("TCL: next -> res", res);
          //@ts-ignore
          const scanJob = res.data.newScanJob;
          if (
            scanJob.status === "created" ||
            scanJob.status === "cashDrawerOpen"
          ) {
            that.pushScanJobToQueue(scanJob);
          }
        },
        error(err) {
          console.error("err", err);
        },
      });
  }

  updatePrintJob(printJobId, status, printJobType, errorReason) {
    this.apolloClient
      .mutate({
        mutation: UPDATE_PRINT_JOB_MUTATION,
        variables: {
          printJobId,
          status,
          printJobType,
          errorReason,
          resolution: "",
        },
      })
      .then((res) => {
        console.log("Printed", res.data.pos.updatePrintJob);
      })
      .catch((e) => {
        console.log("Receipt printing error", e);
      });
  }

  updateScanJob(scanJobId, status, cardRfid) {
    this.apolloClient
      .mutate({
        mutation: UPDATE_SCAN_JOB_MUTATION,
        variables: {
          scanJobId,
          status,
          cardRfid,
        },
      })
      .then((res) => {})
      .catch((e) => {
        console.log("Scanning error printing error", e);
      });
  }

  updateAccessRecord(accessRecordId, cardRfid) {
    return this.apolloClient
      .mutate({
        mutation: UPDATE_ACCESS_RECORD_MUTATION,
        variables: {
          accessRecordId,
          cardRfid,
        },
      })
      .then((res) => {})
      .catch((e) => {
        console.log("An error occured with updating the access record", e);
      });
  }
  clearQueue(printJob) {
    const redisId = this.printJobToRedis[printJob.id];
    if (
      redisId &&
      (printJob.printJobType === "receipt" ||
        printJob.printJobType === "cashout")
    ) {
      this.mq.deleteMessage("receiptPrintJobs", redisId, (messageId) =>
        console.log("deleted receipt print job with id", messageId)
      );
    } else if (redisId && printJob.printJobType === "passMedia") {
      this.mq.deleteMessage("cardPrintJobs", redisId, (messageId) =>
        console.log("deleted card print job with id", messageId)
      );
    }
  }

  pushScanJobToQueue(scanJob) {
    this.mq.sendMessage(
      "scanJobs",
      JSON.stringify({
        id: scanJob.id,
        accessRecordId: scanJob.accessRecordId,
        status: scanJob.status,
      }),
      (messageId) => {
        this.scanJobToRedis[scanJob.id] = messageId;
        console.log("Sent scan job with id", messageId);
      }
    );
  }

  pushPrintJobToQueue(printJob) {
    if (
      printJob.printJobType === "receipt" ||
      printJob.printJobType === "cashout"
    ) {
      this.mq.sendMessage(
        "receiptPrintJobs",
        JSON.stringify({
          id: printJob.id,
          data: printJob.printData,
          printJobType: printJob.printJobType,
        }),
        (messageId) => {
          this.printJobToRedis[printJob.id] = messageId;
          console.log("Sent receipt print job with id", messageId);
        }
      );
    } else if (printJob.printJobType === "passMedia") {
      setTimeout(() => {
        this.mq.sendMessage(
          "cardPrintJobs",
          JSON.stringify({
            id: printJob.id,
            accessRecordId: printJob.accessRecordId,
            data: printJob.printData,
          }),
          (messageId) => {
            this.printJobToRedis[printJob.id] = messageId;
            console.log("Sent card print job with id", messageId);
          }
        );
      }, 50);
    } else if (printJob.printJobType === "shippingLabel") {
      setTimeout(() => {
        this.mq.sendMessage(
          "labelPrintJobs",
          JSON.stringify({
            id: printJob.id,
            data: printJob.printData,
          }),
          (messageId) => {
            this.printJobToRedis[printJob.id] = messageId;
            console.log("Sent label print job with id", messageId);
          }
        );
      }, 50);
    }
  }
}

export default WebSocket;
