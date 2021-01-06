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
    function NodePrint(ws, qName) {
        this.ws = ws;
        this.qName = qName;
        this.currentJobId = null;
        this.numTimesCheckedPrinted = 0;
    }
    /** Handle data, if no ID, end the Job */
    NodePrint.prototype.print = function (printJobData) {
        /**start the print */
        this.beginPrinting(printJobData);
    };
    NodePrint.prototype.beginPrinting = function (printJobData) { };
    /** Remove the message from the queue and remove id from local state */
    /**Check every 500ms if a Job Is is set, then reseive a message from the specified Queue , Print it */
    // public checkPrintJobs() {
    //   this.intervalId = setInterval(() => {
    //     if (
    //       this.currentJobId ||
    //       (this.state.currentState !== "idle" && this.qName !== "scanJobs")
    //     ) {
    //       return;
    //     }
    //     this.mq.receiveMessage(this.qName, (message) => {
    //       if (!message) {
    //         this.currentJobId = null;
    //         return;
    //       }
    //       this.state.updateState(this.qName);
    //       //@ts-ignore
    //       this.print(message);
    //     });
    //   }, 1200);
    // }
    NodePrint.prototype.stop = function () {
        if (this.intervalId) {
            clearInterval(this.intervalId);
        }
    };
    return NodePrint;
}());
exports.default = NodePrint;
