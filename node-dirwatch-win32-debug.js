var DW = require('./build/Debug/node-dirwatch-win32').DirectoryWatcher;
var events = require('events');

inherits(DW, events.EventEmitter);

// extend prototype
function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}

var watcher = new DW('C:/');
watcher.on('change', (event) => {
  console.log('event!', event);
});

watcher.on('error', (err) => {
  console.log(err.message);
});

module.exports.DirectoryWatcher = DW;