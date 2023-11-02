#ifndef LOGGING_H
#define LOGGING_H

#include "easylogging++.h"


namespace el{
    static void configureClientLogger(){
        
        el::Configurations defaultConf;
        defaultConf.setToDefault();
        defaultConf.setGlobally(el::ConfigurationType::Format, "[%datetime] [%level]: %msg");
        defaultConf.setGlobally(el::ConfigurationType::Filename, "../logs/Clientlog-%datetime.log");
        defaultConf.setGlobally(el::ConfigurationType::ToFile, "true");
        defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
        el::Loggers::reconfigureLogger("default", defaultConf);
    }
}





#endif // LOGGING_H



