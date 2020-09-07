const RedisSMQTest = require("rsmq");
const rsmqTest = new RedisSMQTest({
  host: "127.0.0.1",
  port: 6379,
  ns: "rsmq",
});
console.log("rsmqTest", rsmqTest);
rsmqTest.createQueue({ qname: "test" }, (err, resp) => {
  console.log("createQueue -> resp", resp);
  console.log("createQueue -> err", err);
  if (err) {
    console.error(err);
    return;
  }
});
rsmqTest.listQueues((err, queues) => {
  console.log("err", err);
  console.log("deleteQueueIfExists -> queues", queues);
  if (err) {
    console.error(err);

    return;
  }
});
