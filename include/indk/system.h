/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_SYSTEM_H
#define INTERFERENCE_SYSTEM_H

#include <mutex>
#include <condition_variable>
#include <indk/computer.h>

namespace indk {
    typedef std::tuple<std::string, std::string, void*, void*, int> LinkDefinition;
    typedef std::vector<LinkDefinition> LinkList;

    class System {
    public:
        static bool isSynchronizationNeeded();

        /**
        * Set compute backend.
        * @param Backend Compute backend value.
        * @param Parameter Custom parameter.
        */
        static void setComputeBackend(int Backend, int Parameter = 0);

        /**
         * Set library verbosity level.
         * @param VL New verbosity level value.
         */
        static void setVerbosityLevel(int);

        /**
         * Get current compute backend.
         * @return Pointer of current compute backend object.
         */
        static indk::Computer* getComputeBackend();

        /**
         * Get current compute backend.
         * @return ID of current compute backend.
         */
        static int getComputeBackendKind();

        /**
         * Get current verbosity level.
         * @return Verbosity level value.
         */
        static int getVerbosityLevel();

        /**
         * Get current compute backend parameter. The parameter can be set as an argument to the setComputeBackend method. It is always zero for indk::ComputeBackends::Default backend.
         * @return Backend parameter.
         */
        static int getComputeBackendParameter();

        /**
         * Compute backends enum.
         */
        typedef enum {
            /// Native CPU compute backend.
            Default,
            /// Native CPU multithread compute backend. You can set the number of threads by `parameter` argument of setComputeBackend method.
            Multithread,
            /// OpenCL compute backend.
            OpenCL
        } ComputeBackends;
    };

    class Event {
    public:
        Event(): m_bEvent(false) {}
        ~Event() = default;
        bool doWaitTimed(int);
        bool doWait();
        void doNotifyOne();
    private:
        std::mutex m_oMutex;
        std::condition_variable m_oConditionVariable;
        bool m_bEvent;
    };


}

#endif //INTERFERENCE_SYSTEM_H
