#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text)
{
   if(!pos.IsValid()){
        throw InvalidPositionException("invalid position");
       
    }
    table.resize(std::max(pos.row+1,(int)table.size()));
    table[pos.row].resize(std::max(pos.col+1,(int)table[pos.row].size()));
   
    auto &ptr = table[pos.row][pos.col];
    if (!ptr) {
        ptr = std::make_unique<Cell>(*this);
    }
    
    ptr->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid()){
        throw InvalidPositionException("invalid position");
    }
    if (pos.row >= (int)table.size() || pos.col >= (int)table[pos.row].size()) {
        return nullptr;
    }
    return const_cast<Cell*>(table[pos.row][pos.col].get());
}
CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()){
        throw InvalidPositionException("invalid position");
    }
    if (pos.row >= (int)table.size() || pos.col >= (int)table[pos.row].size()) {
        return nullptr;
    }
    return table[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid()){
        throw InvalidPositionException("invalid position");
    }
    
    if (pos.row<(int)table.size() && pos.col<(int)table[pos.row].size()) {
        
        auto &ptr = table[pos.row][pos.col];
        if (ptr){
            ptr.get()->Clear();
            ptr.reset(nullptr);
        }
    }else return;
    
}

Size Sheet::GetPrintableSize() const {
    Size size;
    for(int row = 0; row <(int)table.size(); ++row) {
        for(int col =(int)table[row].size()-1; col>=0; --col) {
            auto &ptr = table[row][col];
            if (ptr) {
                if (!ptr->GetText().empty()) {
                    size.rows = std::max(size.rows, row + 1);
                    size.cols = std::max(size.cols, col + 1);
                    break;
                }
            }
        }
    }
    return size;
}

void Sheet::PrintValues(std::ostream &output) const {
    
    auto size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            if (col < (int)table[row].size()) {
                auto& ptr= table[row][col];
                if (ptr) {
                    auto val=ptr.get()->GetValue();
                    std::visit([&output](auto& a) {output << a; }, val);
                }
            }
        }
        output << '\n';
    }

}


void Sheet::PrintTexts(
        std::ostream &output) const {
    auto size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            if (col < (int)table[row].size()) {
                auto &ptr = table[row][col];
                if (ptr) {
                    output <<ptr.get()->GetText();
                }
            }
        }
        output << '\n';
    }
}

const Cell* Sheet::GetCellPtr(Position pos) const {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");

    if (pos.row >= (int)table.size() || pos.col >= (int)table[pos.row].size()) {
        return nullptr;
    }
    return table[pos.row][pos.col].get();
}

Cell* Sheet::GetCellPtr(Position pos) {
    return const_cast<Cell*>(
        static_cast<const Sheet&>(*this).GetCellPtr(pos));
}



std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
