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
import ws from "ws";
const isProd = process.env.IS_PRODUCTION === "1";
let backendUrl = isProd
  ? "entabeni-api.herokuapp.com"
  : "entabeni-api-staging.herokuapp.com";
const isPreProd = process.env.IS_PRODUCTION === "2";
if (isPreProd) {
  backendUrl = "pre-production-api.herokuapp.com";
}
const websocketUrl = `wss://${backendUrl}/cable`;

const deviceMac = process.env.DEVICE_MAC || "CA:2D:E9:8D:17:67";
const password =
  process.env.TERMINAL_PASSWORD || "973595bf280d548eb8455d4f2d131561";
const envPrintTerminalId =
  process.env.PRINT_TERMINAL_ID || "c9fff07d-5470-44a1-ad96-2c05872078ea";
const frontendUrl =
  `https://${backendUrl}/?frontEndUrl=https://${process.env.FRONTEND_URL}/` ||
  "https://entabeni-api-staging.herokuapp.com/?frontEndUrl=https://pos-demo.entabeni.tech/";

const SIGN_IN_TERMINAL_MUTATION = gql`
  mutation SignInTerminal(
    $deviceMac: String!
    $printTerminalId: String!
    $password: String!
  ) {
    pos {
      signInTerminal(
        deviceMac: $deviceMac
        printTerminalId: $printTerminalId
        password: $password
      ) {
        success
        authToken
      }
    }
  }
`;

class TokenService {
  apolloClient = new ApolloClient({
    cache: new InMemoryCache(),
    link: createHttpLink({
      uri: `/graphql`, // Your GraphQL endpoint
    }),
  });
  printJobToRedis: any;
  scanJobToRedis: any;
  constructor() {}

  initClient(baseUrl: string) {
    const authLink = new ApolloLink((operation, forward) => {
      operation.setContext(() => ({
        headers: {
          authorization: null,
        },
      }));
      return forward(operation);
    });

    // Create regular NetworkInterface by using apollo-client's API:
    const httpLink = new HttpLink({
      uri: `${baseUrl}/graphql`, // Your GraphQL endpoint
      fetch,
    });

    // Create TokenService client
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

  connect(callback: (token: string, baseUrl: string) => void) {
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
        console.log("TCL: connect -> res", res);
        const baseUrl = res.baseUrl;
        this.login(baseUrl, callback);
      })
      .catch(function(error) {
        console.log("error connecting to get baseUrl");
        setTimeout(() => {
          //@ts-ignore
          this.connect();
        }, 15000);
      });
  }

  login(baseUrl: string, callback: (token: string, baseUrl: string) => void) {
    this.initClient(baseUrl);

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
        console.log("login -> token", token);
        callback(token, baseUrl);
        //@ts-ignore
      })
      .catch((error) => {
        console.log("TCL: login -> error", error);
      });
  }
}

export default TokenService;
