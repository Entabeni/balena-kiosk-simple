const pathTestCard = require("path");
const fs = require("fs");
import printer from "node-native-printer";
printer.setPrinter("DYMO_LabelWriter_450");
fs.unlink("log.txt", function() {
  function mpTest(relPath) {
    return pathTestCard.resolve(__dirname, relPath);
  }

  const addressJSON =
    '{"address1":"610 Clematis House Test Cnr Test Road and Smith Street","address2":"Apt 426","state":"FL","zip":"33401","country":"US","city":"West Palm Beach"}';
  const { address1, address2, state, zip, country, city } = JSON.parse(
    addressJSON
  );

  const finalString = address1 +', '+ address2 + "\n" + city + ', '+ state + '\n' + country + '\n' + zip

  console.log("finalString", finalString);

  fs.appendFile("log.txt", finalString, function(err) {
    if (err) {
    console.log("err", err)
    } else {
      console.log("here");
      printer.print(mpTest("log.txt"), {
        landscape: true,
        InputSlot: "Left",
        pageSize: "24_mm__1___Label__Auto_",
      });
    }
  });
});
