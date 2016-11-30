#include "DirectoryWatcher.h"

using namespace v8;
using namespace std;

DirectoryWatcher::DirectoryWatcher(wstring dir)
{
  EventWorker::Run(dir.c_str(), (void*)this);
}

DirectoryWatcher::~DirectoryWatcher()
{
  EventWorker::Stop();
}
