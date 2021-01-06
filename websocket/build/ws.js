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
var apollo_client_1 = __importDefault(require("apollo-client"));
//@ts-ignore
var apollo_link_1 = require("apollo-link");
//@ts-ignore
var apollo_link_error_1 = require("apollo-link-error");
//@ts-ignore
var apollo_link_http_1 = require("apollo-link-http");
var apollo_utilities_1 = require("apollo-utilities");
//@ts-ignore
var ActionCableLink_1 = __importDefault(require("graphql-ruby-client/subscriptions/ActionCableLink"));
var jwt_decode_1 = __importDefault(require("jwt-decode"));
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
var PRINT_JOBS_SUBSCRIPTION = apollo_boost_1.gql(templateObject_1 || (templateObject_1 = __makeTemplateObject(["\n  subscription onNewPrintJob($printTerminalId: String!) {\n    newPrintJob(printTerminalId: $printTerminalId) {\n      id\n    }\n  }\n"], ["\n  subscription onNewPrintJob($printTerminalId: String!) {\n    newPrintJob(printTerminalId: $printTerminalId) {\n      id\n    }\n  }\n"])));
var PRINT_JOBS_QUERY = apollo_boost_1.gql(templateObject_2 || (templateObject_2 = __makeTemplateObject(["\n  query printJob($id: String!) {\n    pos {\n      printJob(id: $id) {\n        id\n        printData\n        printJobType\n        resolution\n        status\n        accessRecordId\n        printTerminalId\n        saleId\n      }\n    }\n  }\n"], ["\n  query printJob($id: String!) {\n    pos {\n      printJob(id: $id) {\n        id\n        printData\n        printJobType\n        resolution\n        status\n        accessRecordId\n        printTerminalId\n        saleId\n      }\n    }\n  }\n"])));
var SCAN_JOBS_SUBSCRIPTION = apollo_boost_1.gql(templateObject_3 || (templateObject_3 = __makeTemplateObject(["\n  subscription onNewScanJob($printTerminalId: String!) {\n    newScanJob(printTerminalId: $printTerminalId) {\n      id\n      status\n      accessRecordId\n    }\n  }\n"], ["\n  subscription onNewScanJob($printTerminalId: String!) {\n    newScanJob(printTerminalId: $printTerminalId) {\n      id\n      status\n      accessRecordId\n    }\n  }\n"])));
var UPDATE_SCAN_JOB_MUTATION = apollo_boost_1.gql(templateObject_4 || (templateObject_4 = __makeTemplateObject(["\n  mutation UpdateScanJobMutation(\n    $scanJobId: String!\n    $status: String!\n    $cardRfid: String\n  ) {\n    pos {\n      updateScanJob(id: $scanJobId, status: $status, cardRfid: $cardRfid) {\n        id\n        status\n        accessRecordId\n      }\n    }\n  }\n"], ["\n  mutation UpdateScanJobMutation(\n    $scanJobId: String!\n    $status: String!\n    $cardRfid: String\n  ) {\n    pos {\n      updateScanJob(id: $scanJobId, status: $status, cardRfid: $cardRfid) {\n        id\n        status\n        accessRecordId\n      }\n    }\n  }\n"])));
var UPDATE_ACCESS_RECORD_MUTATION = apollo_boost_1.gql(templateObject_5 || (templateObject_5 = __makeTemplateObject(["\n  mutation UpdateAccessRecordMutation(\n    $accessRecordId: String!\n    $cardRfid: String!\n  ) {\n    pos {\n      updateAccessRecord(id: $accessRecordId, cardRfid: $cardRfid) {\n        id\n      }\n    }\n  }\n"], ["\n  mutation UpdateAccessRecordMutation(\n    $accessRecordId: String!\n    $cardRfid: String!\n  ) {\n    pos {\n      updateAccessRecord(id: $accessRecordId, cardRfid: $cardRfid) {\n        id\n      }\n    }\n  }\n"])));
var WebSocket = /** @class */ (function () {
    function WebSocket() {
        // mq: any;
        this.apolloClient = new apollo_client_1.default({
            cache: new apollo_boost_1.InMemoryCache(),
            link: apollo_link_http_1.createHttpLink({
                uri: "/graphql",
            }),
        });
        // this.state = state;
        // this.mq = mq;
        this.printJobToRedis = [];
        this.scanJobToRedis = [];
    }
    WebSocket.prototype.initClient = function (token, baseUrl) {
        var authLink = new apollo_link_1.ApolloLink(function (operation, forward) {
            operation.setContext(function () { return ({
                headers: {
                    authorization: token ? token : null,
                },
            }); });
            return forward(operation);
        });
        // Create regular NetworkInterface by using apollo-client's API:
        var httpLink = new apollo_boost_1.HttpLink({
            uri: baseUrl + "/graphql",
            fetch: node_fetch_1.default,
        });
        // Create WebSocket client
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
    WebSocket.prototype.subscribe = function (token, baseUrl, receiptPrintObj, cardScanObj, cardPrintPObj) {
        this.initClient(token, baseUrl);
        var tokenDecoded = jwt_decode_1.default(token);
        var that = this;
        this.apolloClient
            .subscribe({
            query: PRINT_JOBS_SUBSCRIPTION,
            variables: {
                printTerminalId: tokenDecoded.printTerminalId,
            },
            //@ts-ignore
            error: function (err) {
                console.error("err", err);
            },
        })
            .subscribe({
            next: function (res) {
                var printJob = res.data.newPrintJob.id;
                that.apolloClient
                    .query({
                    query: PRINT_JOBS_QUERY,
                    variables: {
                        id: printJob,
                    },
                })
                    .then(function (_a) {
                    var printJobQuery = _a.data;
                    var printJobQueryData = printJobQuery.pos.printJob;
                    if (printJobQueryData.status === "created") {
                        if (printJobQueryData.printJobType === "receipt" ||
                            printJobQueryData.printJobType === "cashout") {
                            receiptPrintObj.beginPrinting(printJobQuery);
                        }
                        else if (printJobQueryData.printJobType === "passMedia") {
                            cardPrintPObj.print(printJobQuery);
                        }
                        else if (printJobQueryData.printJobType === "shippingLabel") {
                            receiptPrintObj.beginPrinting(printJobQuery);
                        }
                    }
                })
                    .catch(function (e) {
                    console.log("Scanning error printing error", e);
                });
            },
            error: function (err) {
                console.error("err", err);
            },
        });
        this.apolloClient
            .subscribe({
            query: SCAN_JOBS_SUBSCRIPTION,
            variables: {
                printTerminalId: tokenDecoded.printTerminalId,
            },
        })
            .subscribe({
            next: function (res) {
                console.log("TCL: next -> res", res);
                //@ts-ignore
                var scanJob = res.data.newScanJob;
                if (scanJob.status === "created" ||
                    scanJob.status === "cashDrawerOpen") {
                    cardScanObj.openScanner(scanJob);
                }
            },
            error: function (err) {
                console.error("err", err);
            },
        });
    };
    WebSocket.prototype.updateScanJob = function (scanJobId, status, cardRfid) {
        this.apolloClient
            .mutate({
            mutation: UPDATE_SCAN_JOB_MUTATION,
            variables: {
                scanJobId: scanJobId,
                status: status,
                cardRfid: cardRfid,
            },
        })
            .then(function (res) { })
            .catch(function (e) {
            console.log("Scanning error printing error", e);
        });
    };
    WebSocket.prototype.updateAccessRecord = function (accessRecordId, cardRfid) {
        return this.apolloClient
            .mutate({
            mutation: UPDATE_ACCESS_RECORD_MUTATION,
            variables: {
                accessRecordId: accessRecordId,
                cardRfid: cardRfid,
            },
        })
            .then(function (res) { })
            .catch(function (e) {
            console.log("An error occured with updating the access record", e);
        });
    };
    return WebSocket;
}());
exports.default = WebSocket;
var templateObject_1, templateObject_2, templateObject_3, templateObject_4, templateObject_5;
