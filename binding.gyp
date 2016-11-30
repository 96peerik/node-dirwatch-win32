{
  "targets": [
    {
      "target_name": "node-dirwatch-win32",
      "sources": [
        "src/DirectoryWatcher.h",
        "src/DirectoryWatcher.cc",
        "src/Utils.h",
        "src/Utils.cc",
        "src/EventWorker.h"
      ],
      "libraries": [
      ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ]
    }
  ]
}
