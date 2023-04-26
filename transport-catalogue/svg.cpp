#include "svg.h"

#include <iomanip>

namespace svg {

    using namespace std::literals;

    // Переопределение вывода цвета
    std::ostream& operator<<(std::ostream& out, const Color& color) {
        out << visit(ColorGenerator{}, color);
        return out;
    }


    // ------Line cap & Line join ------

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        switch (line_cap)
        {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        case StrokeLineCap::SQUARE:
            out << "square";
        default:
            break;
        }

        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
        switch (line_join)
        {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        default:
            break;
        }
        return out;
    }

    // ---------- Object ------------------

    // Отрисовка(формирование результирующей строки)
    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }


    // ---------- Circle ------------------

    // Задаёт координаты центра (атрибуты x и y)
    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;

        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);

        out << "/>"sv;
    }


    // ---------- Polyline ------------------

    // Задаёт координаты опорной точки (атрибуты x и y)
    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;

        bool is_first = true;
        for (auto& point : points_) {
            if (is_first) {
                out << point.x << ',' << point.y;
                is_first = false;
            }
            else {

                out << ' ' << point.x << ',' << point.y;
            }
        }
        out << '\"';

        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);

        out << " />"sv;
    }


    // ---------- Text ------------------

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;

        out << "<text"sv;

        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;

        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;

        out << " font-size=\""sv << font_size_ << "\""sv;

        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }

        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }

        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);

        out << ">"sv;

        for (char i : data_) {
            switch (i)
            {
            case '"':
                out << "&quot;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            default:
                out << i;
                break;
            }
        }

        out << "</text>"sv;
    }


    // ---------- Document ------------------

    // Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (const auto& object : objects_) {
            out << "  "sv;
            object->Render(out);
        }

        out << "</svg>"sv;
    }

}  // namespace svg