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
var action_cable_1 = __importDefault(require("action-cable"));
//@ts-ignore
// import { gql, HttpLink, InMemoryCache } from "apollo-boost";
//@ts-ignore
// import ApolloClient from "apollo-client";
//@ts-ignore
var apollo_link_1 = require("apollo-link");
//@ts-ignore
var apollo_link_error_1 = require("apollo-link-error");
//@ts-ignore
// import { createHttpLink } from "apollo-link-http";
//@ts-ignore
var getmac_1 = __importDefault(require("getmac"));
var apollo_utilities_1 = require("apollo-utilities");
var ActionCableLink_1 = __importDefault(require("graphql-ruby-client/subscriptions/ActionCableLink"));
var jwt_decode_1 = __importDefault(require("jwt-decode"));
var node_localstorage_1 = require("node-localstorage");
var ws_1 = __importDefault(require("ws"));
var gql = require("graphql-tag");
var ApolloClient = require("apollo-client").ApolloClient;
var fetch = require("node-fetch");
var createHttpLink = require("apollo-link-http").createHttpLink;
// const setContext = require("apollo-link-context").setContext;
var InMemoryCache = require("apollo-cache-inmemory").InMemoryCache;
var isProd = process.env.IS_PRODUCTION === "1";
var deviceMac = getmac_1.default();
console.log("deviceMac", deviceMac);
var isPreProd = process.env.IS_PRODUCTION === "2";
var backendUrl;
if (isProd) {
    backendUrl = "entabeni-api.herokuapp.com";
}
else if (isPreProd) {
    backendUrl = "pre-production-api.herokuapp.com";
}
else {
    backendUrl = "entabeni-api-staging.herokuapp.com";
}
var websocketUrl = "wss://" + backendUrl + "/cable/";
var envPrintTerminalId = process.env.PRINT_TERMINAL_ID || "c9fff07d-5470-44a1-ad96-2c05872078ea";
var password = process.env.TERMINAL_PASSWORD || "973595bf280d548eb8455d4f2d131561";
console.log("envPrintTerminalId", envPrintTerminalId);
var frontendUrl = "https://" + backendUrl + "/?frontEndUrl=https://" + process.env.FRONTEND_URL + "/" ||
    "https://entabeni-api-staging.herokuapp.com/?frontEndUrl=https://pos-demo.entabeni.tech/";
console.log("frontendUrl", frontendUrl);
var apiKey = isProd
    ? "335654d2600faead9936251ea066f4a9"
    : "b04f110e-9ae0-4018-8954-4b59de0663e9";
var localStorage = new node_localstorage_1.LocalStorage("./terminal");
var PRINT_JOBS_SUBSCRIPTION = gql(templateObject_1 || (templateObject_1 = __makeTemplateObject(["\n  subscription onNewPrintJob($printTerminalId: String!) {\n    newPrintJob(printTerminalId: $printTerminalId) {\n      id\n      printData\n      printJobType\n      resolution\n      status\n      accessRecordId\n      printTerminalId\n      saleId\n    }\n  }\n"], ["\n  subscription onNewPrintJob($printTerminalId: String!) {\n    newPrintJob(printTerminalId: $printTerminalId) {\n      id\n      printData\n      printJobType\n      resolution\n      status\n      accessRecordId\n      printTerminalId\n      saleId\n    }\n  }\n"])));
var SCAN_JOBS_SUBSCRIPTION = gql(templateObject_2 || (templateObject_2 = __makeTemplateObject(["\n  subscription onNewScanJob($printTerminalId: String!) {\n    newScanJob(printTerminalId: $printTerminalId) {\n      id\n      status\n      accessRecordId\n    }\n  }\n"], ["\n  subscription onNewScanJob($printTerminalId: String!) {\n    newScanJob(printTerminalId: $printTerminalId) {\n      id\n      status\n      accessRecordId\n    }\n  }\n"])));
exports.GET_SALE_DETAILS_BY_ID = gql(templateObject_3 || (templateObject_3 = __makeTemplateObject(["\n  query sale($id: String!) {\n    pos {\n      sale(id: $id) {\n        saleLineItems {\n          guest {\n            id\n            fullName\n            email\n          }\n        }\n      }\n    }\n  }\n"], ["\n  query sale($id: String!) {\n    pos {\n      sale(id: $id) {\n        saleLineItems {\n          guest {\n            id\n            fullName\n            email\n          }\n        }\n      }\n    }\n  }\n"])));
var SIGN_IN_TERMINAL_MUTATION = gql(templateObject_4 || (templateObject_4 = __makeTemplateObject(["\n  mutation SignInTerminal(\n    $deviceMac: String!\n    $password: String!\n    $printTerminalId: String!\n  ) {\n    pos {\n      signInTerminal(\n        deviceMac: $deviceMac\n        password: $password\n        printTerminalId: $printTerminalId\n      ) {\n        success\n        authToken\n      }\n    }\n  }\n"], ["\n  mutation SignInTerminal(\n    $deviceMac: String!\n    $password: String!\n    $printTerminalId: String!\n  ) {\n    pos {\n      signInTerminal(\n        deviceMac: $deviceMac\n        password: $password\n        printTerminalId: $printTerminalId\n      ) {\n        success\n        authToken\n      }\n    }\n  }\n"])));
var UPDATE_SCAN_JOB_MUTATION = gql(templateObject_5 || (templateObject_5 = __makeTemplateObject(["\n  mutation UpdateScanJobMutation(\n    $scanJobId: String!\n    $status: String!\n    $cardRfid: String\n  ) {\n    pos {\n      updateScanJob(id: $scanJobId, status: $status, cardRfid: $cardRfid) {\n        id\n        status\n        accessRecordId\n      }\n    }\n  }\n"], ["\n  mutation UpdateScanJobMutation(\n    $scanJobId: String!\n    $status: String!\n    $cardRfid: String\n  ) {\n    pos {\n      updateScanJob(id: $scanJobId, status: $status, cardRfid: $cardRfid) {\n        id\n        status\n        accessRecordId\n      }\n    }\n  }\n"])));
var UPDATE_ACCESS_RECORD_MUTATION = gql(templateObject_6 || (templateObject_6 = __makeTemplateObject(["\n  mutation UpdateAccessRecordMutation(\n    $accessRecordId: String!\n    $cardRfid: String!\n  ) {\n    pos {\n      updateAccessRecord(id: $accessRecordId, cardRfid: $cardRfid) {\n        id\n      }\n    }\n  }\n"], ["\n  mutation UpdateAccessRecordMutation(\n    $accessRecordId: String!\n    $cardRfid: String!\n  ) {\n    pos {\n      updateAccessRecord(id: $accessRecordId, cardRfid: $cardRfid) {\n        id\n      }\n    }\n  }\n"])));
var WebSocket = /** @class */ (function () {
    function WebSocket(mq, state) {
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
        console.log("WebSocket -> initClient -> baseUrl", baseUrl);
        var httpLink = new createHttpLink({
            uri: baseUrl + "/graphql",
            fetch: fetch,
        });
        // Create WebSocket client
        console.log("WebSocket -> initClient -> websocketUrl", websocketUrl);
        var cable = websocketUrl
            ? action_cable_1.default.createConsumer(websocketUrl, ws_1.default)
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
        var cache = new InMemoryCache({
            dataIdFromObject: function (object) { return object.id || null; },
        });
        // Finally, create your ApolloClient instance with the modified network interface
        //@ts-ignore
        //@ts-ignore
        this.apolloClient = new ApolloClient({
            link: apollo_link_1.from([errorLink, authLink, link]),
            cache: cache,
        });
    };
    WebSocket.prototype.connect = function () {
        var _this = this;
        var token = localStorage.getItem("jwt");
        fetch(frontendUrl, {
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
            var baseUrl = res.baseUrl;
            if (token) {
                _this.subscribe(token, "" + baseUrl);
            }
            else {
                _this.loginThenSubscribe("" + baseUrl);
            }
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
    WebSocket.prototype.loginThenSubscribe = function (baseUrl) {
        var _this = this;
        this.initClient(null, baseUrl);
        // address.mac(function(err, addr) {
        //@ts-ignore
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
            localStorage.setItem("jwt", token);
            //@ts-ignore
            _this.subscribe(token);
        })
            .catch(function (error) { });
        // });
    };
    WebSocket.prototype.subscribe = function (token, baseUrl) {
        this.initClient(token, baseUrl);
        var tokenDecoded = jwt_decode_1.default(token);
        var that = this;
        //@ts-ignore
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
                if ((printJob.status === "created" ||
                    printJob.resolution === "retry") &&
                    printJob.resolution !== "cancel") {
                    if (printJob.status === "created" &&
                        printJob.resolution === "retry") {
                        //@ts-ignore
                        that.state.idle();
                    }
                    that.pushPrintJobToQueue(printJob);
                }
                if (printJob.resolution === "cancel" &&
                    printJob.status !== "complete" &&
                    printJob.status !== "processing") {
                    that.clearQueue(printJob);
                }
            },
            error: function (err) {
                console.error("err", err);
            },
        });
        //@ts-ignore
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
        console.log("updateScanJob -> cardRfid", cardRfid);
        console.log("updateScanJob -> status", status);
        console.log("updateScanJob -> scanJobId", scanJobId);
        //@ts-ignore
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
    WebSocket.prototype.updateAccessRecord = function (accessRecordId, cardRfid) { };
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
var templateObject_1, templateObject_2, templateObject_3, templateObject_4, templateObject_5, templateObject_6;
