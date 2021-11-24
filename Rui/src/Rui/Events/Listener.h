#pragma once

#include "Event.h"

namespace Rui {
    class Listener {
        public:
        virtual ~Listener() {};

        virtual void OnEvent(Event& event) = 0;
    };
}