{
  "targets": [
    {
      "target_name": "node-dirwatch-win32",
      "sources": [
        "src/DirectoryWatcher.h",
        "src/DirectoryWatcher.cc",
        "src/V8Utils.h",
        "src/V8Utils.cc",
      ],
      "libraries": [
      ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ]
    }
  ]
}
