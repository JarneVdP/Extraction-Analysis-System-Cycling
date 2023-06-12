import Toybox.Activity;
import Toybox.Lang;
import Toybox.Time;
import Toybox.WatchUi;

class GarminRTDView extends WatchUi.SimpleDataField {
  var _bcs_profile;
  function initialize(bcs) {
    SimpleDataField.initialize();
    label = "RTD";
    _bcs_profile = bcs;

    _bcs_profile.pair();
  }

  //Initiate variables and their previous values. To not send too much data, we only send data when it changes and if it's their turn to be sent.
  var turn = 0;
  var HR, location, power, distance, speed, batterypercent, timenow, altitude;
  var HRprev, locationprev, powerprev, distanceprev, speedprev, timeprev, altitudeprev;
  var comma = new [0]b;
  var commatoo = new [0]b;
  var LonstackB = new [0]b;
  var LatstackB = new [0]b;
  var DiststackB = new [0]b;
  var AltstackB = new [0]b;
  var PowerstackB = new [0]b;

  var batterypercentprev = 1;
  private var powerhex = 0x03;
  private var altitudehex = 0x06;
  private var lasttimer = 0;
  private var tokmh = 3.6;
  private var displaytext;

  function compute(
    info as Activity.Info
  ) as Numeric or Duration or String or Null {
    //Compute gets called every second. depending on the turn variable different data gets added to the queue
    //Depending on the connection status, the first element of the queue gets sent to the ESP-32

    if (!_bcs_profile.isPaired()) {
      displaytext = "not connected";
      return displaytext;}
      
    displaytext = "connected";

    //test: send twice
    if (info has :timerTime) {
      if (_bcs_profile.connectedtimer == 0) {  //don't send too much data
        _bcs_profile.sendstack();
      }
    }


    var myTime = System.getClockTime();
    // System.println(myTime.hour.format("%02d") + ":" +myTime.min.format("%02d"));
    timenow = myTime.hour.format("%02d") + "." +myTime.min.format("%02d");
    timeprev = (timeprev == null) ? myTime.hour.format("%02d") + "." + myTime.min.format("%02d") : timeprev;

    if (turn == 0 && !timenow.equals(timeprev)) {
      // System.println("Time: " + timenow + " prev: " + timeprev);
      timeprev = timenow;

      comma = _bcs_profile.toArray(timenow, ".");
      var TimestackB = [0x00]b;   //starting with 0 means time
      for (var i = 0; comma[i] != null; i++){
        var tempbyte = new [1]b;  //if I make it length 1 with UINT8
        tempbyte.encodeNumber(comma[i].toNumber() ,Lang.NUMBER_FORMAT_UINT8,{:endianness=>Lang.ENDIAN_LITTLE});   //UINT8 is 1 byte
        TimestackB.addAll(tempbyte);
      }
      _bcs_profile.addmessage(TimestackB);
    }

    if (info has :currentHeartRate && turn == 1 && info.currentHeartRate != null) {
      var HRstackB = [0x01]b;   //starting with 1 means Heart Rate
      var tempbyte = new [1]b;  //if I make it length 1 with UINT8
      tempbyte.encodeNumber(info.currentHeartRate ,Lang.NUMBER_FORMAT_UINT8,{:endianness=>Lang.ENDIAN_LITTLE});   //UINT8 is 1 byte
      HRstackB.addAll(tempbyte);

      _bcs_profile.addmessage(HRstackB);
    }

    //info.currentLocationAccuracy: GPS accuracy values range from 0-4. A value of 0 indicates an accuracy value is not available, while a value of 4 indicates a good GPS fix. We go for 3 and 4 accuracy
    if (info has :currentLocation && info.currentLocation != null && info.currentLocationAccuracy > 2 && turn == 2) {
      location = info.currentLocation.toDegrees();
      locationprev = (locationprev == null) ? info.currentLocation.toDegrees() : locationprev;

      if (locationprev[0] != null && locationprev[1] != null
         &&(!location[0].format("%.4f").equals(locationprev[0].format("%.4f")) || !location[1].format("%.4f").equals(locationprev[1].format("%.4f")))){
        locationprev = location;

        comma = _bcs_profile.toArray(location[0].format("%.4f"), ".");  //Latitude  now 42.7693
        commatoo = _bcs_profile.toArray(location[1].format("%.4f"), ".");  //longitude  now -1.6331

        if (comma[0].toNumber() < 0) {comma[0] = comma[0].toNumber() * -1; LatstackB = [0x20]b;}  //starting with 0x20 = 32  means negative latitude
        else { LatstackB = [0x02]b;}   //starting with 2 means latitude
        if (commatoo[0].toNumber() < 0) {commatoo[0] = commatoo[0].toNumber() * -1; LonstackB = [0x2A]b;}  //starting with 0x2A = 42 means negative longitude
        else { LonstackB = [0x0C]b;}   //starting with 0x0C = 12 means longitude

        for (var i = 0; comma[i] != null; i++){
          var tempbyte = new [1]b;  //if I make it length 1 with UINT8
          var tempbytetoo = new [1]b;  //if I make it length 1 with UINT8

          if (comma[i].toNumber() > 255){ comma[i+1] = comma[i].toNumber() % 100; comma[i] = comma[i].toNumber() / 100;}
          if (commatoo[i].toNumber() > 255){ commatoo[i+1] = commatoo[i].toNumber() % 100; commatoo[i] = commatoo[i].toNumber() / 100;}

          tempbyte.encodeNumber(comma[i].toNumber() ,Lang.NUMBER_FORMAT_UINT8,{:endianness=>Lang.ENDIAN_LITTLE});   //UINT8 is 1 byte
          tempbytetoo.encodeNumber(commatoo[i].toNumber() ,Lang.NUMBER_FORMAT_UINT8,{:endianness=>Lang.ENDIAN_LITTLE});   //UINT8 is 1 byte
          LatstackB.addAll(tempbyte);
          LonstackB.addAll(tempbytetoo);
        }
      _bcs_profile.addmessage(LatstackB);
      _bcs_profile.addmessage(LonstackB);
      }
      displaytext = location[0].format("%.4f") + " " + location[1].format("%.4f");
    }
    
    if (info has :currentPower && turn == 3 && power != null && power != powerprev) { //not yet tested
      power = info.currentPower;
      powerprev = power;
      
      while (power > 255){
        power = power - 255;
        powerhex = powerhex + 0x0A;
      }
      var PowerstackB = [powerhex]b;   //powerhex starts at 0x03 and jumps with increments of 10 (0x0A). calulation is: if secondvalue == 3, power =  receivedpower + firstvalue*255
      var tempbyte = new [1]b;  
      tempbyte.encodeNumber(power ,Lang.NUMBER_FORMAT_UINT8,{:endianness=>Lang.ENDIAN_LITTLE});   //UINT8 is 1 byte
      AltstackB.addAll(tempbyte);
      _bcs_profile.addmessage(PowerstackB);
    }

    if (info has :currentSpeed && turn == 4 && info.currentSpeed != null && info.currentSpeed != 0.0) {

      comma = _bcs_profile.toArray((info.currentSpeed * tokmh).format("%.2f"), ".");  //convert to km/h with 2 values behind the comma should not be more than 255 :D
      var SpeedstackB = [0x04]b;   //starting with 4 means speed
      for (var i = 0; comma[i] != null; i++){
        var tempbyte = new [1]b;  //if I make it length 1 with UINT8
        tempbyte.encodeNumber(comma[i].toNumber() ,Lang.NUMBER_FORMAT_UINT8,{:endianness=>Lang.ENDIAN_LITTLE});   //UINT8 is 1 byte
        SpeedstackB.addAll(tempbyte);
      }
      _bcs_profile.addmessage(SpeedstackB);
    }

    if (info has :elapsedDistance && turn == 5 && info.elapsedDistance != null && info.elapsedDistance != distanceprev) {
      distanceprev = info.elapsedDistance;

      comma = _bcs_profile.toArray((info.elapsedDistance / 1000).format("%.2f"), "."); //convert meters to kilometer with 2 values behind the comma
      if (comma[0].toNumber() > 255){ comma[0] = comma[0].toNumber() - 255; DiststackB = [0x0F]b; } // 0x0F = 15 means distance > 255. add 255 to the value at the receiver
      else{ DiststackB = [0x05]b; }  //starting with 5 means distance

      for (var i = 0; comma[i] != null; i++){
        var tempbyte = new [1]b;  //if I make it length 1 with UINT8
        tempbyte.encodeNumber(comma[i].toNumber() ,Lang.NUMBER_FORMAT_UINT8,{:endianness=>Lang.ENDIAN_LITTLE});   //UINT8 is 1 byte
        DiststackB.addAll(tempbyte);
      }
      _bcs_profile.addmessage(DiststackB);
    }

    if (info has :altitude && turn == 6 && info.altitude != null && info.altitude.toNumber() != altitudeprev) {
      altitudeprev = info.altitude.toNumber();
      altitude = info.altitude;
      altitudehex = 0x06; //reset
      while (altitude > 255){
        altitude = altitude - 255;
        altitudehex = altitudehex + 0x0A; // +10
      }

      var AltstackB = [altitudehex]b;   //powerhex starts at 0x03 and jumps with increments of 10 (0x0A). calulation is: if secondvalue == 3, power =  receivedpower + firstvalue*255
      var tempbyte = new [1]b;  
      tempbyte.encodeNumber(altitude ,Lang.NUMBER_FORMAT_UINT8,{:endianness=>Lang.ENDIAN_LITTLE});   //UINT8 is 1 byte
      AltstackB.addAll(tempbyte);
      _bcs_profile.addmessage(AltstackB);
    }

    batterypercent = System.getSystemStats().battery.toNumber(); 
    if (turn == 7 && batterypercent != batterypercentprev) {
      batterypercentprev = batterypercent.toNumber();

      var BattstackB = [0x07]b;   //starting with 6 means battery of E530
      var tempbyte = new [1]b;  //if I make it length 1 with UINT8
      tempbyte.encodeNumber(batterypercent.toNumber() ,Lang.NUMBER_FORMAT_UINT8,{:endianness=>Lang.ENDIAN_LITTLE});   //UINT8 is 1 byte
      BattstackB.addAll(tempbyte);

      _bcs_profile.addmessage(BattstackB);
    }

      turn = (turn == 7) ? 0 : turn + 1;
      //Count down from 10 seconds to zero. When zero, stable connection, start data sending.
      _bcs_profile.connectedtimer = (_bcs_profile.connectedtimer != 0) ? _bcs_profile.connectedtimer - 1 : 0;

      //Timer shouldn't overflow at 2,635,318 as it's a 32 bit signed integer so overflow at 2,147,483,647
      if (info has :timerTime) {
        System.println("Timer:" + info.timerTime + " interval: " + (info.timerTime - lasttimer).toString());
        if (_bcs_profile.connectedtimer == 0) {  //don't send too much data   info.timerTime - lasttimer > 1500 &&
          _bcs_profile.sendstack();
          lasttimer = info.timerTime;
          return "sending";
        }
      }

    return Lang.format("$1$  $2$", [info.timerTime.toString(), displaytext]);
  }
}