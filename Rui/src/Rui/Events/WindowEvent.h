#pragma once

#include "Event.h"

namespace Rui {
    class WindowEvent : public Event {
        public:

        EVENT_CLASS_CATEGORY(EventCategoryApplication)
        protected:
        WindowEvent() {}
    };

    class WindowResizedEvent : public WindowEvent {
        public:
        WindowResizedEvent(int w, int h)
                : WindowEvent(), width(w), height(h) {
        }

        inline int GetWidth() const { return width; }
        inline int GetHeight() const { return height; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "WindowResizedEvent: " << width << ", " << height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResized)
        private:
        int width;
        int height;
    };

    class WindowClosedEvent : public WindowEvent {
        public:
        WindowClosedEvent()
                : WindowEvent() {
        }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "WindowClosedEvent: ";
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowClosed)
        private:
    };
}