var async = require('async');
var noble = require('noble');
var xbmc = require('xbmc-ws');

// Settings:
var peripheralUuid = process.argv[2] || '478386a605e01721968f1cb425e69e97';
var serviceUuid = 'ffe0';
var characteristicUuid = 'ffe1';
var volume = 30; // initial volume if no volumeChange detected yet

// Mac requirements to enable/disable caffeine and play a sound on connect
var sound = require('osx-system-sounds');
var applescript = require('applescript');

// Connect to xbmc/kodi enter IP of xbmc/kodi host here
connection = xbmc('192.168.1.20', 9090);

// Register error handler
connection.on('error', function xbmc_error(error) {
  console.error("XBMC error:", error);
});

// Open caffeine and activate it
var script = 'tell application "Caffeine" to turn on';
applescript.execString(script, function(err) {
  if (err) {
    console.log('ERROR: Could not enable caffeine');
  }
  else {
    console.log('Caffeine enabled');
  }
});

function quitCaffeineAndExit() {
  var script = 'tell application "Caffeine" to quit';
  applescript.execString(script, function(err) {
    if (err) {
      console.log('ERROR: Could not quit caffeine');
    }
    else {
      console.log('Caffeine closed');
    }
    // Exit for real
    process.exit();
  });
}
// Quit caffeine no matter what
process.on('exit', quitCaffeineAndExit);
process.on('SIGINT', quitCaffeineAndExit);
process.on('uncaughtException', quitCaffeineAndExit);

var volume = 30;

function receivedCommand(data) {
  data = data + "";
  console.log('DATA: ' + data);
  if(data.indexOf("volup") !== -1) {
    volume += 2;
    checkAndUpdateVolume();
  }
  else if(data.indexOf("voldown") !== -1) {
    volume -= 2;
    checkAndUpdateVolume();
  }
  else if(data.indexOf("ok") !== -1) {
    connection.run('Input.Select')();
  }
  else if(data.indexOf("back") !== -1) {
    connection.run('Input.Back')();
  }
  else if(data.indexOf("goup") !== -1) {
    connection.run('Input.Up')();
  }
  else if(data.indexOf("godown") !== -1) {
    connection.run('Input.Down')();
  }
  else if(data.indexOf("playpause") !== -1) {
    try{
      connection.run('Player.PlayPause')(1);
    } catch(e) {
      console.log('PlayPause failed: ', e);
    }

  }
  else {
    console.log('Invalid data: ' + data + '#');
  }
}

function checkAndUpdateVolume() {
  if(volume > 100) {
    volume = 100;
  } else if(volume < 0) {
    volume = 0;
  }
  console.log('Volume: ' + volume);
  if(isNaN(parseInt(volume))) {
    console.log('Could not partse number: ' + volume);
  }
  connection.run('Application.SetVolume')(parseInt(volume));
}

connection.on('Application.OnVolumeChanged', function(data) {
  // Update volume
  volume = data.data.volume;
});


console.log('Looking for ' + peripheralUuid + ' with service ' + serviceUuid);

noble.on('stateChange', function(state) {
  if (state === 'poweredOn') {
    noble.startScanning();
  } else {
    noble.stopScanning();
  }
});

noble.on('discover', function(peripheral) {
  if (peripheral.uuid === peripheralUuid) {
    noble.stopScanning();

    console.log('Connected to ' + peripheral.advertisement.localName + ' # ' + peripheral.uuid);

    peripheral.on('disconnect', function() {
      console.log('DEVICE DISCONNECTED');
      console.log('Starting discovery...');
      noble.startScanning();
    });
    // Connect
    peripheral.connect(function(error) {
      if(error) console.log(error);
      // Discover services
      peripheral.discoverServices([], function(error, services) {
        if(error) console.log(error);
        // Check service until we find ffe0
        for(i = 0; i < services.length; i++) {
          var service = services[i];
          console.log('Service (UUID: ' + service.uuid + '): ' + service.name);
          if(service.uuid == serviceUuid) {
            console.log('FOUND SERVICE '+ service.uuid +'. Discover characteristics ...');
            service.discoverCharacteristics([], function(error, characteristics) {
              if(error) console.log(error);
              // Check each characteristic until we find ffe1
              for(x = 0; x < characteristics.length; x ++) {
                var characteristic = characteristics[x];
                if(characteristic.uuid == characteristicUuid) {
                  console.log('FOUND CHARACTERISTIC ' + characteristic.uuid);
                  console.log('Start listening');
                  // Listen for read event
                  characteristic.on('read', receivedCommand);
                  // Get notification when new data arrives
                  characteristic.notify(true, function(error) {
                    if(error) console.log(error);
                  });
                  // Play sound to indicate we are connected
                  sound.play(6);
                }
              }
            }); // Discover characteristics
          }   // Service found
        }   // Iterate over service
      }); // Discover service
    }); // Connect
  }
  else {
    console.log('FOUND unkown peripheral with UUID: ' + peripheral.uuid);
  }
});