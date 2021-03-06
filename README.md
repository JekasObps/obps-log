# ObpsLog
Simple c++ logging library written for my personal use. Welcome to everyone who interests in.

## usage:
  // create instance of logger in any scope. Compatible with std::cout, etc...
  auto logger = obps::ObpsLog::CreateLog(std::cout, obps::LogLevel::INFO);

  // template Log method is flexible, variable argument and supports iostream formatting
  // 	!!!caution: many different signatures may cause code bloating!
  logger->Log(obps::LogLevel::INFO, "[App]", ": value is ", 5);

  
  // one logger can be attached to another so all messages will pass through both of them.
  logger->Attach(another_logger);


