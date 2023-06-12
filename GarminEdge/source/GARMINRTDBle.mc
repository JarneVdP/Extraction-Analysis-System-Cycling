using Toybox.System;
using Toybox.WatchUi;
using Toybox.BluetoothLowEnergy as Ble;
using Toybox.Lang;
using Toybox.Timer;

class BLE_Connect_Send {
  var _bledelegate;
  var _blePaired = false;
  var _device;

  const UUID_ESP_S = Ble.stringToUuid("5fafc201-2510-459e-8fcc-c5c9c331914b"); //Service
  const UUID_ESP_C = Ble.stringToUuid("5eb5483e-2510-4688-b7f5-ea07361b26a8"); // Characteristic

  const NAME_ESP_S = "ESP-S";
  const NAME_ESP_N = "ESP-N";

  //timer
  var connectedtimer = -1;

  //Message stack
  var messagestack = [];
  var writeBusy = false;

  var BLE_CS_Profile = {
    //Bluetooth Low Energy Connect&Send Profile
    :uuid => UUID_ESP_S,
    :characteristics => [
      {
        :uuid => UUID_ESP_C,
        :descriptors => [Ble.cccdUuid()],
      },
    ],
  };

  function initialize() {
    System.println("BLE_CS.initialize");
    // Make the registerProfile call
    _bledelegate = new BLE_CS_Delegate(self);
    Ble.setDelegate(_bledelegate);
    Ble.registerProfile(BLE_CS_Profile);
  }

  function isPaired() {
    System.println("BLE_CS.isPaired " + _blePaired);
    return _blePaired;
  }

  function pair() {
    if (!_blePaired) {
      System.println("BLE_CS.pairing");

      Ble.setScanState(Ble.SCAN_STATE_SCANNING);
    }
  }

  function unpair() {
    if (_device != null) {
      Ble.unpairDevice(_device);
      _blePaired = false;
      _device = null;
    }
    WatchUi.requestUpdate();
  }

  //Send message to the stack and to the BLE device
  function addmessage(msg) {
    messagestack.add(msg);
    System.println("Stack: " + messagestack);
  }

  function clearfromstack() {
    messagestack = messagestack.slice(1, messagestack.size());
    // System.println("Stack after cleaning: " + messagestack);
  }

  function sendstack() as Void {
    System.println("size " +messagestack.size() +" writeBusy " +writeBusy +" _device " +_device);
    if (messagestack.size() > 0 && !writeBusy && _device != null) {
      try {
        writeBusy = true;
        var ble_charact = _device.getService(UUID_ESP_S).getCharacteristic(UUID_ESP_C);
        System.println("BLE_CS: Writing " + messagestack[0]);

        ble_charact.requestWrite(messagestack[0], {:writeType => Ble.WRITE_TYPE_DEFAULT});
      } catch (ex) {
        System.println("Stack communication error: " + ex.getErrorMessage());
        ex.printStackTrace();
      }
      System.println("Cleaning stack");
      clearfromstack();
      writeBusy = false;
    }
  }

  function toArray(string, splitter) {
    var array = new [4];
    var index = 0;
    var location;
    do {
      location = string.find(splitter);
      if (location != null) {
        array[index] = string.substring(0, location);
        string = string.substring(location + 1, string.length());
        index++;
      }
    } while (location != null);
    array[index] = string;

    var result = new [index];
    for (var i = 0; i < index; i++) {
      result = array;
    }
    return result;
  }
}

class BLE_CS_Delegate extends Ble.BleDelegate {
  private var _bcs_profile;

  public function initialize(bcs) {
    BleDelegate.initialize();
    _bcs_profile = bcs;
  }

  function onConnectedStateChanged(device, state) {
    if (state == Ble.CONNECTION_STATE_CONNECTED) {
      System.println("BLE_CS: Connected to " + device.getName());
      _bcs_profile._blePaired = true;
      _bcs_profile._device = device;
      //start the connectedtimer
      _bcs_profile.connectedtimer = 5;
    } else {
      System.println("BLE_CS: Disconnected from " + device.getName());
      _bcs_profile._blePaired = false;
      _bcs_profile._device = null;
      _bcs_profile.connectedtimer = -1;
    }
    WatchUi.requestUpdate();
  }

  function onScanResults(scanResults) {
    for (
      var result = scanResults.next();
      result != null;
      result = scanResults.next()
    ) {
      if (result instanceof Ble.ScanResult) {
        System.println(
          "Scan BLE_CS: " +
            result.getDeviceName() +
            " " +
            result.getServiceUuids()
        );

        if (contains(result.getServiceUuids(), _bcs_profile.UUID_ESP_S)) {
          System.println(
            "Found BLE_CS: " +
              result.getDeviceName() +
              " " +
              result.getServiceUuids()
          );
          Ble.setScanState(Ble.SCAN_STATE_OFF);
          _bcs_profile._device = Ble.pairDevice(result);
          _bcs_profile._blePaired = true;
          WatchUi.requestUpdate();
        }
      }
    }
  }

  function onCharacteristicWrite(characteristic, status) {
    System.println(
      "BLE_CS: onCharacteristicWrite " + characteristic + "comm ready?" + status
    );
  }

  private function contains(iter, obj) {
    for (var uuid = iter.next(); uuid != null; uuid = iter.next()) {
      System.println("Checking " + uuid + " against " + obj + " " + (uuid.equals(obj)));
      System.println("Iter" + iter);
      if (uuid.equals(obj)) {
        System.println("Match " + obj);
        return true;
      }
    }
    return false;
  }
}
