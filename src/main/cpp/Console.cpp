#include "Console.hpp"

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include "util/EscapeCodes.hpp"
#include "util/ArrowKey.hpp"
#include "widgets/Widget.hpp"

// TODO: Add signals for console events
Blame::Console::Console() {
    this->clear();
    this->moveCaret(std::cout, 0, 0);
    std::cout << Blame::Util::EscapeCodes::caretOff();

    this->buffer_list.push_back(&front_buffer);
    this->buffer_list.push_back(&back_buffer);

    this->exit.store(false);
    this->has_flipped.store(true);

    struct winsize size{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    this->width = size.ws_col;
    this->height = size.ws_row;

    this->client_area.left = 0;
    this->client_area.top = 0;
    this->client_area.right = this->width;
    this->client_area.bottom = this->height;

    struct termios term{};
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ICANON;
    term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void Blame::Console::mainLoop() {
    char first;
    char second;
    char third;

    bool find_second_third;

    this->focused_widget = this->focus_order[0];
    this->focused_widget->focus();

    std::thread draw_thread(&Blame::Console::drawLoop, this);

    while (!this->exit.load()) {
        find_second_third = true;
        std::cout.flush();

        std::cin >> first;

        if (first == 'q') {
            this->quit();
        }

        if (first == 'f') {
            this->incrementFocus();
            find_second_third = false;
        }

        if (first == 'e') {
            auto new_widget = dynamic_cast<Blame::Widgets::Widget *>(this->focused_widget);
            new_widget->activate();
            find_second_third = false;
        }

        if (find_second_third) {
            std::cin >> second;
            std::cin >> third;
        }

        if (first == 27 && second == 91) {
            if (third == 65) {
                // std::cout << "UP";
                for (auto widget : this->widget_list) {
                    widget->arrowKey(Blame::Util::ArrowKey::UP);
                }
            }
            else if (third == 66) {
                // std::cout << "DOWN";
                for (auto widget : this->widget_list) {
                    widget->arrowKey(Blame::Util::ArrowKey::DOWN);
                }
            }
            else if (third == 67) {
                // std::cout << "RIGHT";
                for (auto widget : this->widget_list) {
                    widget->arrowKey(Blame::Util::ArrowKey::RIGHT);
                }
            }
            else if (third == 68) {
                // std::cout << "LEFT";
                for (auto widget : this->widget_list) {
                    widget->arrowKey(Blame::Util::ArrowKey::LEFT);
                }
            }
        }
    }
}

void Blame::Console::drawLoop() {
    std::chrono::milliseconds current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    std::chrono::milliseconds start_time = current_time;
    int frame_count = 0;

    while (!this->exit.load()) {
        frame_count++;
        current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());

        // this->setTitle("Current Time: " + std::to_string(current_time.count()) + " Start Time:" + std::to_string(start_time.count()) + " Final: " + std::to_string(current_time.count() - start_time.count()));
        if (current_time.count() - start_time.count() > 0.25f) {
            frame_count = 0;

            start_time = current_time;

            // this->setTitle("--------------------------------");
            if (this->has_flipped.load()) {
                this->has_flipped.exchange(false);

                if (this->buffer_list[this->current_buffer]->rdbuf() != std::cout.rdbuf()) {
                    this->redraw();
                    this->flipBuffers();
                }
            }
        }
    }
}

void Blame::Console::clear() {
    std::cout << Blame::Util::EscapeCodes::reset();
    printf("\033c");
}

void Blame::Console::redraw() {
    // TODO: Only redraw the cells that have been changed, don't clear the whole thing
    this->clear();

    this->moveCaret(std::cout, 0, 0);

    // TODO: Maybe add a check for if they need to be redrawn?
    for (auto widget : this->widget_list) {
        // The widgets are added as listeners, since the Widget class is incomplete at the time of headers
        // meaning they can't be used for the typing of the list, so we have to cast them to widgets...
        // which they might not be
        auto new_widget = dynamic_cast<Blame::Widgets::Widget *>(widget);

        if (new_widget->is_redrawn.load()) {
            new_widget->redraw();

            if (!new_widget->children.empty()) {
                for (auto child : new_widget->children) {
                    child->redraw();
                }
            }
        }
    }
}

void Blame::Console::setTitle(std::string str) {
    std::cout << "\033]2;" << str.c_str() << "\007";
}

void Blame::Console::moveCaret(std::ostream &stream, int column, int row) {
    stream << "\033[" << row << ";" << column << "H";
}
