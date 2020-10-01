import TokenService from "./tokenService";
import WebSocket from "./ws";
import StateMachine from "./stateMachine";
import MessageQueue from "./messageQueue";
import CardPrinter from "./printers/CardPrinter";
import ReceiptPrinter from "./printers/ReceiptPrinter";
import CardScanner from "./printers/CardScanner";
import LabelPrinter from "./printers/LabelPrinter";
// export {};

function main() {
  const state = new StateMachine();
  const mq = new MessageQueue();
  const ws = new WebSocket(mq, state);
  const ts = new TokenService();
  const receiptPrintObj = new ReceiptPrinter(mq, ws, "receiptPrintJobs", state);
  const cardScanObj = new CardScanner(mq, ws, "scanJobs", state);
  const cardPrintPObj = new CardPrinter(mq, ws, "cardPrintJobs", state);
  const lablePrintObj = new LabelPrinter(mq, ws, "labelPrintJobs", state);

  mq.deleteQueueIfExists("receiptPrintJobs", (resp1) => {
    if (resp1) {
      mq.deleteQueueIfExists("cardPrintJobs", (resp2) => {
        if (resp2) {
          mq.deleteQueueIfExists("labelPrintJobs", (resp3) => {
            if (resp3) {
              mq.deleteQueueIfExists("scanJobs", (resp4) => {
                if (resp4) {
                  mq.createQueue("receiptPrintJobs", (resp5) => {
                    if (resp5) {
                      mq.createQueue("cardPrintJobs", (resp6) => {
                        if (resp6) {
                          mq.createQueue("labelPrintJobs", (resp7) => {
                            if (resp7) {
                              mq.createQueue("scanJobs", (resp8) => {
                                if (resp8) {
                                  ts.connect((token, baseUrl) => {
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
