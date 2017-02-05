var DW = require('./build/Debug/node-dirwatch-win32').DirectoryWatcher;
var events = require('events');

inherits(DW, events.EventEmitter);

// extend prototype
function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}

module.exports.DirectoryWatcher = DW;