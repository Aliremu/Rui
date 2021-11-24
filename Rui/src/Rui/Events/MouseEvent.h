#pragma once

#include "Event.h"

namespace Rui {
    class MouseEvent : public Event {
        public:

        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
        protected:
        MouseEvent() {}
    };

    class MousePressEvent : public MouseEvent {
        public:
        MousePressEvent(int button, int x, int y)
                : MouseEvent(), button(button), x(x), y(y) {
        }

        inline int GetButton() const { return button; }
        inline int GetX() const { return x; }
        inline int GetY() const { return y; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "MousePressEvent: " << x << ", " << y;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
        private:
        int button;
        int x;
        int y;
    };

    class MouseReleaseEvent : public MouseEvent {
        public:
        MouseReleaseEvent(int button, int x, int y)
                : MouseEvent(), button(button), x(x), y(y) {
        }

        inline int GetButton() const { return button; }
        inline int GetX() const { return x; }
        inline int GetY() const { return y; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "MouseReleaseEvent: " << x << ", " << y;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonReleased)
        private:
        int button;
        int x;
        int y;
    };

    class MouseMoveEvent : public MouseEvent {
        public:
        MouseMoveEvent(int xrel, int yrel)
                : MouseEvent(), x(xrel), y(yrel) {
        }

        inline int GetX() const { return x; }
        inline int GetY() const { return y; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "MouseMoveEvent: " << x << ", " << y;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseMoved)
        private:
        int x;
        int y;
    };
}