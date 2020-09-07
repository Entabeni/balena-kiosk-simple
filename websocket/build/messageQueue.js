"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var RedisSMQ = require("rsmq");
var MessageQueue = /** @class */ (function () {
    function MessageQueue() {
        // Connect to Redis and create printJobs queue if it doesn't exist
        this.rsmq = new RedisSMQ({ host: "127.0.0.1", port: 6379, ns: "rsmq" });
    }
    MessageQueue.prototype.deleteQueue = function (qName, callback) {
        this.rsmq.deleteQueue({ qname: qName }, function (err, resp) {
            if (err) {
                console.error(err);
                return;
            }
            callback(resp);
        });
    };
    MessageQueue.prototype.deleteQueueIfExists = function (qName, callback) {
        var _this = this;
        this.rsmq.listQueues(function (err, queues) {
            if (err) {
                console.error(err);
                return;
            }
            var found = queues.findIndex(function (element) { return element === qName; });
            if (found === -1) {
                callback(1);
            }
            else {
                _this.rsmq.deleteQueue({ qname: qName }, function (err, resp) {
                    if (err) {
                        console.error(err);
                        return;
                    }
                    callback(resp);
                });
            }
        });
    };
    MessageQueue.prototype.createQueue = function (qName, callback) {
        this.rsmq.createQueue({ qname: qName }, function (err, resp) {
            if (err) {
                console.error(err);
                return;
            }
            callback(resp);
        });
    };
    MessageQueue.prototype.getOrCreateQueue = function (qName, callback) {
        var _this = this;
        this.rsmq.listQueues(function (err, queues) {
            if (err) {
                console.error(err);
                return;
            }
            var found = queues.findIndex(function (element) { return element === qName; });
            if (found === -1) {
                _this.rsmq.createQueue({ qname: qName }, function (err, resp) {
                    callback(resp);
                });
            }
            else {
                callback(1);
            }
        });
    };
    MessageQueue.prototype.sendMessage = function (qName, message, callback) {
        this.rsmq.sendMessage({ qname: qName, message: message }, function (err, resp) {
            if (resp) {
                callback(resp);
            }
            else {
                callback(null);
            }
        });
    };
    MessageQueue.prototype.receiveMessage = function (qName, callback) {
        this.rsmq.receiveMessage({ qname: qName }, function (err, resp) {
            if (resp && resp.id) {
                callback(resp);
            }
            else {
                callback(null);
            }
        });
    };
    MessageQueue.prototype.deleteMessage = function (qName, id, callback) {
        this.rsmq.deleteMessage({ qname: qName, id: id }, function (err, resp) {
            if (resp === 1) {
                callback(true);
            }
            else {
                callback(false);
            }
        });
    };
    return MessageQueue;
}());
exports.default = MessageQueue;
