/////////////////////////////////////////////////////////////////////////////
// Name:        indk/interlink.h
// Purpose:     Interlink service class header
// Author:      Nickolay Babbysh
// Created:     19.07.2023
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_INTERLINK_H
#define INTERFERENCE_INTERLINK_H

#include <string>
#include <thread>
#include <atomic>

namespace indk {
    /**
     * Interlink class.
     */
    class Interlink {
    private:
//        void *LinkedObject;
        void *Input;
        void *Output;
        std::string Host;
        std::string InputPort, OutputPort;
        std::thread Thread;
        std::atomic<bool> Interlinked;
        std::string Structure;

        void doInitInput(int, int);
        void doInitOutput();

        void doSend(const std::string&, const std::string&);
    public:
        Interlink();
        Interlink(int, int timeout);
        void doUpdateStructure(const std::string&);
        void doUpdateModelData(const std::string&);
        void doUpdateMetrics(const std::string&);
        void setStructure(const std::string&);
        std::string getStructure();
        bool isInterlinked();
        ~Interlink();
    };
}

#endif //INTERFERENCE_INTERLINK_H
