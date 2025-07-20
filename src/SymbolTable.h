//
// Created by remsc on 13/06/2025.
//

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <vector>

#include "AST.h"
class StructDefinitionAST;
class BlockAST;
class AST;

using namespace std;


struct SymbolInfo {
    enum SymbolType {
        Function, Structure, Variable, Generic, Type
    };
    std::string type;
    SymbolType metaType;
    bool isStatic;
    bool constant;
};

// Map: struct_name -> { field_name -> SymbolInfo }
using StructFieldMap = std::map<std::string, SymbolInfo>;

class SymbolTable {
    std::vector<std::map<std::string, SymbolInfo>> scopes;
    std::map<std::string, StructFieldMap> knownStructs;

    std::map<std::string, const StructDefinitionAST*> structTemplates;
    set<string> instantiations;

public:
    unique_ptr<BlockAST> generics; // Block holding the monomorphs structure

    SymbolTable() : generics(make_unique<BlockAST>()) {
        enterScope();
        addSymbol("int", {"int", SymbolInfo::Type});
        // TODO : more types
        /*addSymbol("uint", {"unsigned int", SymbolInfo::Type});
        addSymbol("int8", {"char", SymbolInfo::Type});
        addSymbol("uint8", {"unsigned char", SymbolInfo::Type});
        addSymbol("int16", {"short", SymbolInfo::Type});
        addSymbol("uint16", {"unsigned short", SymbolInfo::Type});
        addSymbol("int32", {"int", SymbolInfo::Type});
        addSymbol("uint32", {"unsigned int", SymbolInfo::Type});
        addSymbol("int64", {"long", SymbolInfo::Type});
        addSymbol("uint64", {"unsigned long", SymbolInfo::Type});*/

        addSymbol("float", {"float", SymbolInfo::Type});
        addSymbol("double", {"double", SymbolInfo::Type});
        //addSymbol("ldouble", {"long double", SymbolInfo::Type});

        addSymbol("char", {"char", SymbolInfo::Type});
        //addSymbol("uchar", {"unsigned char", SymbolInfo::Type});
    }

    void enterScope() {
        scopes.emplace_back();
    }

    void exitScope() {
        if (scopes.size() > 1) {
            scopes.pop_back();
        }
    }

    bool addStruct(const std::string& structName, const StructFieldMap& fields) {
        if (knownStructs.contains(structName)) {
            return false; // Déjà définie
        }
        knownStructs[structName] = fields;
        return true;
    }

    bool addTemplate(const string& name, const StructDefinitionAST* ast) {
        if (structTemplates.contains(name)) return false;
        structTemplates[name] = ast;
        return true;
    }

    const StructDefinitionAST* lookupTemplate(const string& name) {
        if (const auto it = structTemplates.find(name); it != structTemplates.end()) {
            return it->second;
        }
        return nullptr;
    }

    void addInstantiation(const string& name) {
        instantiations.insert(name);
    }

    optional<string> findInstantiation(const string& name) {
        if(const auto it = instantiations.find(name); it != instantiations.end()) {
            return name;
        }
        return std::nullopt;
    }

    void registerGeneric(unique_ptr<AST> ast) const;

    std::optional<SymbolInfo> lookupField(const std::string& structName, const std::string& fieldName) {
        if (const auto it = knownStructs.find(structName); it != knownStructs.end()) {
            const auto& fields = it->second;
            if (const auto field_it = fields.find(fieldName); field_it != fields.end()) {
                return field_it->second;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Adds a symbol to the table
     * @return true if success, else false
     */
    bool addSymbol(const std::string& name, const SymbolInfo& info) {
        /*if (global) {
            if (scopes.empty()) return false;
            auto& globalScope = scopes.front();
            if (globalScope.contains(name) && globalScope.at(name).metaType == info.metaType) {
                return false;
            }
            globalScope[name] = info;
            return true;
        }*/
        if (scopes.empty()) return false;
        // Can be added if the meta type is different
        // ex : function named 'a' can be added even if there exists a variable 'a'
        if (scopes.back().contains(name) && scopes.back()[name].metaType == info.metaType) {
            return false;
        }
        scopes.back()[name] = info;
        return true;
    }

    /** Function looking up for a symbol in the current scope.
     * @param name name of the symbol
     * @return SymbolInfo if found, else, nullopt
     */
    std::optional<SymbolInfo> lookupSymbol(const std::string& name) {
        for (auto & scope : std::ranges::reverse_view(scopes)) {
            if (auto symbol = scope.find(name); symbol != scope.end()) {
                return symbol->second;
            }
        }
        return std::nullopt;
    }
};


#endif //SYMBOLTABLE_H
