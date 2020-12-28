"use strict";
var __makeTemplateObject = (this && this.__makeTemplateObject) || function (cooked, raw) {
    if (Object.defineProperty) { Object.defineProperty(cooked, "raw", { value: raw }); } else { cooked.raw = raw; }
    return cooked;
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
//@ts-ignore
global.fetch = require("node-fetch");
var action_cable_node_1 = __importDefault(require("action-cable-node"));
//@ts-ignore
var node_fetch_1 = __importDefault(require("node-fetch"));
//@ts-ignore
var apollo_boost_1 = require("apollo-boost");
//@ts-ignore
var apollo_client_1 = __importDefault(require("apollo-client"));
//@ts-ignore
var apollo_link_1 = require("apollo-link");
//@ts-ignore
var apollo_link_error_1 = require("apollo-link-error");
//@ts-ignore
var getmac_1 = __importDefault(require("getmac"));
//@ts-ignore
var apollo_link_http_1 = require("apollo-link-http");
var apollo_utilities_1 = require("apollo-utilities");
//@ts-ignore
var ActionCableLink_1 = __importDefault(require("graphql-ruby-client/subscriptions/ActionCableLink"));
var ws_1 = __importDefault(require("ws"));
var isProd = process.env.IS_PRODUCTION === "1";
var backendUrl = isProd
    ? "entabeni-api.herokuapp.com"
    : "entabeni-api-staging.herokuapp.com";
var isPreProd = process.env.IS_PRODUCTION === "2";
if (isPreProd) {
    backendUrl = "pre-production-api.herokuapp.com";
}
var websocketUrl = "wss://" + backendUrl + "/cable";
var macType = process.env.IS_WIFI || "eth0";
var deviceMac = getmac_1.default(macType) || "CA:2D:E9:8D:17:67";
var password = process.env.TERMINAL_PASSWORD || "973595bf280d548eb8455d4f2d131561";
var envPrintTerminalId = process.env.PRINT_TERMINAL_ID || "c9fff07d-5470-44a1-ad96-2c05872078ea";
var frontendUrl = "https://" + backendUrl + "/?frontEndUrl=https://" + process.env.FRONTEND_URL + "/" ||
    "https://entabeni-api-staging.herokuapp.com/?frontEndUrl=https://pos-demo.entabeni.tech/";
var SIGN_IN_TERMINAL_MUTATION = apollo_boost_1.gql(templateObject_1 || (templateObject_1 = __makeTemplateObject(["\n  mutation SignInTerminal(\n    $deviceMac: String!\n    $printTerminalId: String!\n    $password: String!\n  ) {\n    pos {\n      signInTerminal(\n        deviceMac: $deviceMac\n        printTerminalId: $printTerminalId\n        password: $password\n      ) {\n        success\n        authToken\n      }\n    }\n  }\n"], ["\n  mutation SignInTerminal(\n    $deviceMac: String!\n    $printTerminalId: String!\n    $password: String!\n  ) {\n    pos {\n      signInTerminal(\n        deviceMac: $deviceMac\n        printTerminalId: $printTerminalId\n        password: $password\n      ) {\n        success\n        authToken\n      }\n    }\n  }\n"])));
var TokenService = /** @class */ (function () {
    function TokenService() {
        this.apolloClient = new apollo_client_1.default({
            cache: new apollo_boost_1.InMemoryCache(),
            link: apollo_link_http_1.createHttpLink({
                uri: "/graphql",
            }),
        });
    }
    TokenService.prototype.initClient = function (baseUrl) {
        var authLink = new apollo_link_1.ApolloLink(function (operation, forward) {
            operation.setContext(function () { return ({
                headers: {
                    authorization: null,
                },
            }); });
            return forward(operation);
        });
        // Create regular NetworkInterface by using apollo-client's API:
        var httpLink = new apollo_boost_1.HttpLink({
            uri: baseUrl + "/graphql",
            fetch: node_fetch_1.default,
        });
        // Create TokenService client
        var cable = websocketUrl
            ? action_cable_node_1.default.createConsumer(websocketUrl, ws_1.default)
            : null;
        var wsLink = new ActionCableLink_1.default({
            cable: cable,
        });
        var errorLink = apollo_link_error_1.onError(function (_a) {
            var graphQLErrors = _a.graphQLErrors, networkError = _a.networkError, operation = _a.operation;
            if (!!graphQLErrors) {
                graphQLErrors.forEach(function (_a) {
                    var message = _a.message, path = _a.path;
                    return console.log("[GraphQL Error]: Message: " + message + ", Path: " + path);
                });
            }
            if (!!networkError) {
                console.log("[Network error " + operation.operationName + "]: Message: " + networkError.message);
            }
        });
        // Extend the network interface with the WebSocket
        var link = apollo_link_1.split(function (_a) {
            var query = _a.query;
            // @ts-ignore
            var _b = apollo_utilities_1.getMainDefinition(query), kind = _b.kind, operation = _b.operation;
            return kind === "OperationDefinition" && operation === "subscription";
        }, wsLink, httpLink);
        var cache = new apollo_boost_1.InMemoryCache({
            dataIdFromObject: function (object) { return object.id || null; },
        });
        // Finally, create your ApolloClient instance with the modified network interface
        this.apolloClient = new apollo_client_1.default({
            link: apollo_link_1.from([errorLink, authLink, link]),
            cache: cache,
        });
    };
    TokenService.prototype.connect = function (callback) {
        var _this = this;
        node_fetch_1.default(frontendUrl, {
            method: "GET",
            headers: {
                "Content-Type": "application/json",
                "X-Key-Inflection": "camel",
            },
        })
            .then(function (u) {
            return u.json();
        })
            .then(function (res) {
            console.log("TCL: connect -> res", res);
            var baseUrl = res.baseUrl;
            _this.login(baseUrl, callback);
        })
            .catch(function (error) {
            var _this = this;
            console.log("error connecting to get baseUrl");
            setTimeout(function () {
                //@ts-ignore
                _this.connect();
            }, 15000);
        });
    };
    TokenService.prototype.login = function (baseUrl, callback) {
        this.initClient(baseUrl);
        this.apolloClient
            .mutate({
            mutation: SIGN_IN_TERMINAL_MUTATION,
            variables: {
                deviceMac: deviceMac,
                printTerminalId: envPrintTerminalId,
                password: password,
            },
        })
            .then(function (_a) {
            var data = _a.data;
            var token = data.pos.signInTerminal.authToken;
            console.log("login -> token", token);
            callback(token, baseUrl);
            //@ts-ignore
        })
            .catch(function (error) {
            console.log("TCL: login -> error", error);
        });
    };
    return TokenService;
}());
exports.default = TokenService;
var templateObject_1;
