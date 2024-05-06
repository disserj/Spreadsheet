#include "cell.h"
#include "cell.h"
#include "sheet.h"


#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
    :impl_(""),sheet_(sheet)
    {}


void Cell::Set(std::string text) {
    std::regex r("^=[\\w|\\d|\\s|\\.|\\(|\\)|/|+|*|^|=|-]+");
    if(std::regex_match(text,r)){
        if (HasCircularDependency(std::move(std::make_unique<FormulaImpl>(std::move(text.substr(1)),sheet_)))){
            throw CircularDependencyException("");
        }else{
            impl_=std::make_unique<FormulaImpl>(std::move(text.substr(1)),sheet_);
        }
    }else{
        impl_=text;
        return;
    }

    for (auto& c : referenced_cells) {
        c->dependent_cells.erase(this);
    }
    referenced_cells.clear();

    for(auto& pos : std::get<std::unique_ptr<FormulaImpl>>(impl_)->GetReferencedCells()){
        Cell* out = sheet_.GetCellPtr(pos);
        if(!out){
            sheet_.SetCell(pos, "");
            out = sheet_.GetCellPtr(pos);
        }
        referenced_cells.insert(out);
        out->dependent_cells.insert(this);
    }

    InvalidateCache(true);
}

void Cell::Clear() {
    impl_="";
}

Cell::Value Cell::GetValue() const {
    if(std::holds_alternative<std::unique_ptr<FormulaImpl>>(impl_)){
        return std::get<1>(impl_)->GetValue();
    }else if(std::holds_alternative<std::string>(impl_)){
        return  std::get<0>(impl_).front()=='\''?std::get<0>(impl_).substr(1):std::get<0>(impl_);
    }
    return "";
}
std::string Cell::GetText() const {
    std::string res="";
    if(std::holds_alternative<std::unique_ptr<FormulaImpl>>(impl_)){
        res=std::get<std::unique_ptr<FormulaImpl>>(impl_)->GetText();       
    }else if(std::holds_alternative<std::string>(impl_)){
        res=std::get<std::string>(impl_);
    }
    return res;
}

std::vector<Position> Cell::GetReferencedCells() const {
    if(std::holds_alternative<std::unique_ptr<FormulaImpl>>(impl_)){
        return std::get<std::unique_ptr<FormulaImpl>>(impl_)->GetReferencedCells();       
    }
    return {};
}

bool Cell::HasCircularDependency(std::unique_ptr<FormulaImpl>&& some_temporary_value){
 
    std::unordered_set<Cell*> referenced;
    for (const auto& pos : some_temporary_value->GetReferencedCells()) {
        referenced.insert(sheet_.GetCellPtr(pos));
    }

    std::unordered_set<Cell*> visited;
    std::stack<Cell*> to_visit;
    to_visit.push(this);
    while (!to_visit.empty()) {
       auto current = to_visit.top();
        to_visit.pop();
        visited.insert(current);

        if (referenced.find(current) != referenced.end()) return true;
        
        for (auto& cell : current->dependent_cells) {
            if (visited.find(cell) == visited.end()) to_visit.push(cell);
        }
    }

    return false;      
}

void Cell::InvalidateCache(bool flag){
        if(flag || std::get<std::unique_ptr<FormulaImpl>>(impl_)->IsCacheValid()) {
        std::get<std::unique_ptr<FormulaImpl>>(impl_)->ResetCache();
        for (auto i : dependent_cells) {
            i->InvalidateCache();
        }
    }
}
