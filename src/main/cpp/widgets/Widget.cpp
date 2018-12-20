#include <iostream>
#include <optional>
#include "Widget.hpp"
#include "../util/EscapeCodes.hpp"

Blame::Widgets::Widget::Widget(Blame::Console *console, std::optional<Blame::Widgets::Widget*> parent, int column, int row, int width, int height) {
    this->console = console;
    this->parent = parent;

    this->column = column;
    this->row = row;
    this->width = width;
    this->height = height;
}

void Blame::Widgets::Widget::redraw() {
    this->console->moveCaret(this->column, this->row);

    for (int h = 0; h < this->height; h++) {
        this->console->moveCaret(this->column, this->row + h);

        for (int w = 0; w < this->width; w++) {
            // Top Left
            if (w == 0 && h == 0) {
                this->setColours();
                std::cout << this->symbol_top_left;
            }
            // Middle Left
            else if (w == 0 && h > 0 && h < this->height - 1) {
                this->setColours();
                std::cout << this->symbol_middle_left;
            }
            // Bottom Left
            else if (w == 0 && h == this->height - 1) {
                this->setColours();
                std::cout << this->symbol_bottom_left;
            }

            // Top Middle
            if (h == 0) {
                this->setColours();
                std::cout << this->symbol_top_middle;
            }
            // Bottom Middle
            else if (h == this->height - 1) {
                this->setColours();
                std::cout << this->symbol_bottom_middle;
            }
            // Middle Fill
            else {
                std::cout << Blame::Util::EscapeCodes::reset();
                std::cout << this->symbol_middle_fill;
            }

            // Top Right
            if (w == this->width - 1 && h == 0) {
                this->setColours();
                std::cout << this->symbol_top_right;
            }
            // Middle Right
            else if (w == this->width - 1 && h > 0 && h < this->height - 1) {
                this->setColours();
                std::cout << this->symbol_middle_right;
            }
            // Bottom Right
            else if (w == this->width - 1 && h == this->height - 1) {
                this->setColours();
                std::cout << this->symbol_bottom_right;
            }
        }
        std::cout << Blame::Util::EscapeCodes::reset();
        std::cout << std::endl;
    }
}

void Blame::Widgets::Widget::setColours() {
    std::cout << this->colour_border;
    std::cout << this->colour_background;
}