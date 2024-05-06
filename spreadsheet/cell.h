#pragma once

#include "common.h"
#include "formula.h"

#include <variant>
#include <regex>
#include <set>
#include <memory>
#include <optional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell()=default;
    Cell(const Cell&)=delete;
    Cell(Cell&&)=delete;

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

private:  

 class FormulaImpl{
    public:
        FormulaImpl(const std::string& expression,const SheetInterface& sheet)
        :sheet_(sheet)
     {
            formula_ptr = ParseFormula(std::move(expression));    
        }

        Value GetValue() const {
            auto value = formula_ptr->Evaluate(sheet_);
            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            }
            return std::get<FormulaError>(value);
        }

        std::string GetText() const {
            return "=" + formula_ptr->GetExpression();
        }
     
        std::vector<Position> GetReferencedCells() const {
            return formula_ptr->GetReferencedCells();
    }

    bool IsCacheValid(){
        return cache.has_value();
    }
    void ResetCache(){
        cache.reset();
    }
     
    private:
     
        std::unique_ptr<FormulaInterface> formula_ptr;
        const SheetInterface& sheet_;
        mutable std::optional<FormulaInterface::Value> cache;
        
    };   
    
     // impl_ хранит либо пустую строку, либо строку-текст, либо указатель на объект класса FormulaImpl
    std::variant<std::string,std::unique_ptr<FormulaImpl>> impl_; 
    
    Sheet& sheet_;
    std::set<Cell*> dependent_cells;		
    std::set<Cell*> referenced_cells;


    bool HasCircularDependency(std::unique_ptr<FormulaImpl>&& some_value);
    void InvalidateCache(bool flag=false);
    
};