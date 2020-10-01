"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var tokenService_1 = __importDefault(require("./tokenService"));
var ws_1 = __importDefault(require("./ws"));
var stateMachine_1 = __importDefault(require("./stateMachine"));
var messageQueue_1 = __importDefault(require("./messageQueue"));
var CardPrinter_1 = __importDefault(require("./printers/CardPrinter"));
var ReceiptPrinter_1 = __importDefault(require("./printers/ReceiptPrinter"));
var CardScanner_1 = __importDefault(require("./printers/CardScanner"));
var LabelPrinter_1 = __importDefault(require("./printers/LabelPrinter"));
// export {};
function main() {
    var state = new stateMachine_1.default();
    var mq = new messageQueue_1.default();
    var ws = new ws_1.default(mq, state);
    var ts = new tokenService_1.default();
    var receiptPrintObj = new ReceiptPrinter_1.default(mq, ws, "receiptPrintJobs", state);
    var cardScanObj = new CardScanner_1.default(mq, ws, "scanJobs", state);
    var cardPrintPObj = new CardPrinter_1.default(mq, ws, "cardPrintJobs", state);
    var lablePrintObj = new LabelPrinter_1.default(mq, ws, "labelPrintJobs", state);
    mq.deleteQueueIfExists("receiptPrintJobs", function (resp1) {
        if (resp1) {
            mq.deleteQueueIfExists("cardPrintJobs", function (resp2) {
                if (resp2) {
                    mq.deleteQueueIfExists("labelPrintJobs", function (resp3) {
                        if (resp3) {
                            mq.deleteQueueIfExists("scanJobs", function (resp4) {
                                if (resp4) {
                                    mq.createQueue("receiptPrintJobs", function (resp5) {
                                        if (resp5) {
                                            mq.createQueue("cardPrintJobs", function (resp6) {
                                                if (resp6) {
                                                    mq.createQueue("labelPrintJobs", function (resp7) {
                                                        if (resp7) {
                                                            mq.createQueue("scanJobs", function (resp8) {
                                                                if (resp8) {
                                                                    ts.connect(function (token, baseUrl) {
                                                                        ws.subscribe(token, baseUrl);
                                                                        cardScanObj.checkPrintJobs();
                                                                        receiptPrintObj.checkPrintJobs();
                                                                        cardPrintPObj.checkPrintJobs();
                                                                        lablePrintObj.checkPrintJobs();
                                                                    });
                                                                }
                                                            });
                                                        }
                                                    });
                                                }
                                            });
                                        }
                                    });
                                }
                            });
                        }
                    });
                }
            });
        }
    });
}
main();
