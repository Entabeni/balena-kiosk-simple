import { PrintJobMessage } from "./printers/NodePrint";
const RedisSMQ = require("rsmq");

class MessageQueue {
  public rsmq: any;
  constructor() {
    // Connect to Redis and create printJobs queue if it doesn't exist
    this.rsmq = new RedisSMQ({ host: "127.0.0.1", port: 6379, ns: "rsmq" });
  }

  deleteQueue(qName: string, callback: (resp: number) => void) {
    this.rsmq.deleteQueue({ qname: qName }, (err, resp) => {
      if (err) {
        console.error(err);
        return;
      }
      callback(resp);
    });
  }

  deleteQueueIfExists(qName: string, callback: (resp: number) => void) {
    this.rsmq.listQueues((err, queues) => {
      if (err) {
        console.error(err);
        return;
      }
      const found = queues.findIndex((element: string) => element === qName);
      if (found === -1) {
        callback(1);
      } else {
        this.rsmq.deleteQueue({ qname: qName }, (err, resp) => {
          if (err) {
            console.error(err);
            return;
          }
          callback(resp);
        });
      }
    });
  }

  createQueue(qName: string, callback: (resp: number) => void) {
    this.rsmq.createQueue({ qname: qName }, (err, resp) => {
      if (err) {
        console.error(err);
        return;
      }
      callback(resp);
    });
  }

  getOrCreateQueue(qName: string, callback: (resp: number) => void) {
    this.rsmq.listQueues((err, queues) => {
      if (err) {
        console.error(err);
        return;
      }
      const found = queues.findIndex((element: string) => element === qName);
      if (found === -1) {
        this.rsmq.createQueue({ qname: qName }, (err, resp) => {
          callback(resp);
        });
      } else {
        callback(1);
      }
    });
  }

  sendMessage(
    qName: string,
    message: any,
    callback: (resp: number | null) => void
  ) {
    this.rsmq.sendMessage({ qname: qName, message }, (err, resp) => {
      if (resp) {
        callback(resp);
      } else {
        callback(null);
      }
    });
  }

  receiveMessage(
    qName: string,
    callback: (resp: PrintJobMessage | number | null) => void
  ) {
    this.rsmq.receiveMessage({ qname: qName }, (err, resp) => {
      if (resp && resp.id) {
        callback(resp);
      } else {
        callback(null);
      }
    });
  }

  deleteMessage(qName: string, id: any, callback: (resp: boolean) => void) {
    this.rsmq.deleteMessage({ qname: qName, id }, (err, resp) => {
      if (resp === 1) {
        callback(true);
      } else {
        callback(false);
      }
    });
  }
}
export default MessageQueue;
