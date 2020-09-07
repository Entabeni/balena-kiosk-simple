"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.mp = void 0;
var MessageQueue = require("../messageQueue");
var path = require("path");
//@ts-ignore
function mp(relPath) {
    return path.resolve(__dirname, relPath);
}
exports.mp = mp;
var NodePrint = /** @class */ (function () {
    function NodePrint(mq, ws, qName, state) {
        this.mq = mq;
        this.ws = ws;
        this.qName = qName;
        this.finishPrintJob = this.finishPrintJob;
        this.currentJobId = null;
        this.numTimesCheckedPrinted = 0;
        this.state = state;
    }
    /** Handle data, if no ID, end the Job */
    NodePrint.prototype.print = function (printJobData) {
        this.currentJobId = printJobData.id;
        var message = JSON.parse(printJobData.message);
        if (!message.id) {
            this.finishPrintJob(printJobData.id);
            return;
        }
        else {
        }
        /**start the print */
        this.beginPrinting(printJobData);
    };
    NodePrint.prototype.beginPrinting = function (printJobData) { };
    /** Remove the message from the queue and remove id from local state */
    NodePrint.prototype.finishPrintJob = function (printJobId, holdUi) {
        if (holdUi === void 0) { holdUi = false; }
        this.mq.deleteMessage(this.qName, printJobId, function (success) {
            return console.log(success);
        });
        this.currentJobId = null;
        if (!holdUi) {
            this.state.idle();
        }
    };
    /**Check every 500ms if a Job Is is set, then reseive a message from the specified Queue , Print it */
    NodePrint.prototype.checkPrintJobs = function () {
        var _this = this;
        this.intervalId = setInterval(function () {
            if (_this.currentJobId ||
                (_this.state.currentState !== "idle" && _this.qName !== "scanJobs")) {
                return;
            }
            _this.mq.receiveMessage(_this.qName, function (message) {
                if (!message) {
                    _this.currentJobId = null;
                    return;
                }
                _this.state.updateState(_this.qName);
                //@ts-ignore
                _this.print(message);
            });
        }, 800);
    };
    NodePrint.prototype.stop = function () {
        if (this.intervalId) {
            clearInterval(this.intervalId);
        }
    };
    return NodePrint;
}());
exports.default = NodePrint;
