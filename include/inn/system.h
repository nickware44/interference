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
#include "computer.h"

namespace inn {
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
        static inn::Computer* getComputeBackend();

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
        template<typename DurationType>
        bool TimedWait(DurationType const& rTimeout);
        bool doWait();
        void doNotifyOne();
    private:
        std::mutex m_oMutex;
        std::condition_variable m_oConditionVariable;
        bool m_bEvent;
    };


}

#endif //INTERFERENCE_SYSTEM_H
