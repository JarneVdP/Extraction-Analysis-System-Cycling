import Toybox.Application;
import Toybox.Lang;
import Toybox.WatchUi;
using Toybox.BluetoothLowEnergy as Ble;
using Toybox.Timer;

class GarminRTDApp extends Application.AppBase {

     var _bcs_profile;
    function initialize() {
        AppBase.initialize();
    }

    // onStart() is called on application start up
    function onStart(state as Dictionary?) as Void {
        _bcs_profile = new BLE_Connect_Send();
    }

    // onStop() is called when your application is exiting
    function onStop(state as Dictionary?) as Void {
    }

    // Return the initial view of your application here
    function getInitialView() as Array<Views or InputDelegates>? {
        return [ new GarminRTDView(_bcs_profile) ] as Array<Views or InputDelegates>;
    }

    function onSettingsChanged() as Void {
        // WatchUi.requestUpdate();
    }

}

function getApp() as GarminRTDApp {
    return Application.getApp() as GarminRTDApp;
}