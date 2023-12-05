#ifndef LOGGING_H
#define LOGGING_H

#include "easylogging++.h"

namespace el {
static void configureServerLogger() {

    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(el::ConfigurationType::Format,
                            "[%datetime] [%level] %msg");
    defaultConf.setGlobally(el::ConfigurationType::Filename,
                            "../logs/log-%datetime.log");
    defaultConf.setGlobally(el::ConfigurationType::ToFile, "true");
    defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
    el::Loggers::reconfigureLogger("default", defaultConf);

    el::Logger* vanLogger = el::Loggers::getLogger("Van");
    defaultConf.setGlobally(el::ConfigurationType::Format,
                            "[%datetime] [%level] [%logger] %msg");
    el::Loggers::reconfigureLogger("Van", defaultConf);
}
} // namespace el

#endif // LOGGING_H
