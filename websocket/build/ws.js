"use strict";
var __makeTemplateObject = (this && this.__makeTemplateObject) || function (cooked, raw) {
    if (Object.defineProperty) { Object.defineProperty(cooked, "raw", { value: raw }); } else { cooked.raw = raw; }
    return cooked;
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.GET_SALE_DETAILS_BY_ID = void 0;
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
var websocketUrl = "wss://" + backendUrl + "/cable";
var PRINT_JOBS_SUBSCRIPTION = apollo_boost_1.gql(templateObject_1 || (templateObject_1 = __makeTemplateObject(["\n  subscription onNewPrintJob($printTerminalId: String!) {\n    newPrintJob(printTerminalId: $printTerminalId) {\n      id\n      printData\n      printJobType\n      resolution\n      status\n      accessRecordId\n      printTerminalId\n      saleId\n    }\n  }\n"], ["\n  subscription onNewPrintJob($printTerminalId: String!) {\n    newPrintJob(printTerminalId: $printTerminalId) {\n      id\n      printData\n      printJobType\n      resolution\n      status\n      accessRecordId\n      printTerminalId\n      saleId\n    }\n  }\n"])));
var SCAN_JOBS_SUBSCRIPTION = apollo_boost_1.gql(templateObject_2 || (templateObject_2 = __makeTemplateObject(["\n  subscription onNewScanJob($printTerminalId: String!) {\n    newScanJob(printTerminalId: $printTerminalId) {\n      id\n      status\n      accessRecordId\n    }\n  }\n"], ["\n  subscription onNewScanJob($printTerminalId: String!) {\n    newScanJob(printTerminalId: $printTerminalId) {\n      id\n      status\n      accessRecordId\n    }\n  }\n"])));
exports.GET_SALE_DETAILS_BY_ID = apollo_boost_1.gql(templateObject_3 || (templateObject_3 = __makeTemplateObject(["\n  query sale($id: String!) {\n    pos {\n      sale(id: $id) {\n        saleLineItems {\n          guest {\n            id\n            fullName\n            email\n          }\n        }\n      }\n    }\n  }\n"], ["\n  query sale($id: String!) {\n    pos {\n      sale(id: $id) {\n        saleLineItems {\n          guest {\n            id\n            fullName\n            email\n          }\n        }\n      }\n    }\n  }\n"])));
var UPDATE_SCAN_JOB_MUTATION = apollo_boost_1.gql(templateObject_4 || (templateObject_4 = __makeTemplateObject(["\n  mutation UpdateScanJobMutation(\n    $scanJobId: String!\n    $status: String!\n    $cardRfid: String\n  ) {\n    pos {\n      updateScanJob(id: $scanJobId, status: $status, cardRfid: $cardRfid) {\n        id\n        status\n        accessRecordId\n      }\n    }\n  }\n"], ["\n  mutation UpdateScanJobMutation(\n    $scanJobId: String!\n    $status: String!\n    $cardRfid: String\n  ) {\n    pos {\n      updateScanJob(id: $scanJobId, status: $status, cardRfid: $cardRfid) {\n        id\n        status\n        accessRecordId\n      }\n    }\n  }\n"])));
var UPDATE_ACCESS_RECORD_MUTATION = apollo_boost_1.gql(templateObject_5 || (templateObject_5 = __makeTemplateObject(["\n  mutation UpdateAccessRecordMutation(\n    $accessRecordId: String!\n    $cardRfid: String!\n  ) {\n    pos {\n      updateAccessRecord(id: $accessRecordId, cardRfid: $cardRfid) {\n        id\n      }\n    }\n  }\n"], ["\n  mutation UpdateAccessRecordMutation(\n    $accessRecordId: String!\n    $cardRfid: String!\n  ) {\n    pos {\n      updateAccessRecord(id: $accessRecordId, cardRfid: $cardRfid) {\n        id\n      }\n    }\n  }\n"])));
var WebSocket = /** @class */ (function () {
    function WebSocket(mq, state) {
        this.apolloClient = new apollo_client_1.default({
            cache: new apollo_boost_1.InMemoryCache(),
            link: apollo_link_http_1.createHttpLink({
                uri: "/graphql",
            }),
        });
        this.state = state;
        this.mq = mq;
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
    WebSocket.prototype.subscribe = function (token, baseUrl) {
        this.initClient(token, baseUrl);
        var tokenDecoded = jwt_decode_1.default(token);
        console.log("subscribe -> tokenDecoded", tokenDecoded);
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
                console.log("TCL: next -> res", res);
                var printJob = res.data.newPrintJob;
                if (printJob.status === "created") {
                    that.pushPrintJobToQueue(printJob);
                }
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
                    that.pushScanJobToQueue(scanJob);
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
        console.log("TCL: updateAccessRecord -> cardRfid", cardRfid);
        console.log("TCL: updateAccessRecord -> accessRecordId", accessRecordId);
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
    WebSocket.prototype.clearQueue = function (printJob) {
        var redisId = this.printJobToRedis[printJob.id];
        if (redisId &&
            (printJob.printJobType === "receipt" ||
                printJob.printJobType === "cashout")) {
            this.mq.deleteMessage("receiptPrintJobs", redisId, function (messageId) {
                return console.log("deleted receipt print job with id", messageId);
            });
        }
        else if (redisId && printJob.printJobType === "passMedia") {
            this.mq.deleteMessage("cardPrintJobs", redisId, function (messageId) {
                return console.log("deleted card print job with id", messageId);
            });
        }
    };
    WebSocket.prototype.pushScanJobToQueue = function (scanJob) {
        var _this = this;
        this.mq.sendMessage("scanJobs", JSON.stringify({
            id: scanJob.id,
            accessRecordId: scanJob.accessRecordId,
            status: scanJob.status,
        }), function (messageId) {
            _this.scanJobToRedis[scanJob.id] = messageId;
            console.log("Sent scan job with id", messageId);
        });
    };
    WebSocket.prototype.pushPrintJobToQueue = function (printJob) {
        var _this = this;
        if (printJob.printJobType === "receipt" ||
            printJob.printJobType === "cashout") {
            this.mq.sendMessage("receiptPrintJobs", JSON.stringify({
                id: printJob.id,
                data: printJob.printData,
                printJobType: printJob.printJobType,
            }), function (messageId) {
                _this.printJobToRedis[printJob.id] = messageId;
                console.log("Sent receipt print job with id", messageId);
            });
        }
        else if (printJob.printJobType === "passMedia") {
            setTimeout(function () {
                _this.mq.sendMessage("cardPrintJobs", JSON.stringify({
                    id: printJob.id,
                    accessRecordId: printJob.accessRecordId,
                    data: printJob.printData,
                }), function (messageId) {
                    _this.printJobToRedis[printJob.id] = messageId;
                    console.log("Sent card print job with id", messageId);
                });
            }, 50);
        }
        else if (printJob.printJobType === "shippingLabel") {
            setTimeout(function () {
                _this.mq.sendMessage("labelPrintJobs", JSON.stringify({
                    id: printJob.id,
                    data: printJob.printData,
                }), function (messageId) {
                    _this.printJobToRedis[printJob.id] = messageId;
                    console.log("Sent label print job with id", messageId);
                });
            }, 50);
        }
    };
    return WebSocket;
}());
exports.default = WebSocket;
var templateObject_1, templateObject_2, templateObject_3, templateObject_4, templateObject_5;
