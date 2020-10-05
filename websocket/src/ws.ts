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

import { getMainDefinition } from "apollo-utilities";
//@ts-ignore
import ActionCableLink from "graphql-ruby-client/subscriptions/ActionCableLink";
import jwtDecode from "jwt-decode";
import ws from "ws";
import MessageQueue from "./messageQueue";
import StateMachine from "./stateMachine";
const isProd = process.env.IS_PRODUCTION === "1";
let backendUrl = isProd
  ? "entabeni-api.herokuapp.com"
  : "entabeni-api-staging.herokuapp.com";
const isPreProd = process.env.IS_PRODUCTION === "2";
if (isPreProd) {
  backendUrl = "pre-production-api.herokuapp.com";
}
const websocketUrl = `wss://${backendUrl}/cable`;
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
      uri: `/graphql`, // Your GraphQL endpoint
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
      uri: `${baseUrl}/graphql`, // Your GraphQL endpoint
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

  subscribe(token: string, baseUrl: string) {
    this.initClient(token, baseUrl);
    const tokenDecoded = jwtDecode(token);
    console.log("subscribe -> tokenDecoded", tokenDecoded);

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
          if (printJob.status === "created") {
            that.pushPrintJobToQueue(printJob);
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
    console.log("TCL: updateAccessRecord -> cardRfid", cardRfid);
    console.log("TCL: updateAccessRecord -> accessRecordId", accessRecordId);
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
