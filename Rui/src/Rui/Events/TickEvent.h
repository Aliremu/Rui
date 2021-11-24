#pragma once

#include "Event.h"

namespace Rui {
    class TickEvent : public Event {
        public:
        TickEvent() {};
        ~TickEvent() {};

        EVENT_CLASS_TYPE(TickEvent);
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };
}