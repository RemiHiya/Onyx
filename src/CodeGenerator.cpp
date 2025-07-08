//
// Created by remsc on 18/06/2025.
//

#include "CodeGenerator.h"

#include <format>
#include <iostream>
#include <set>

#include "Logger.h"
#include "AST.h"


// Should generate code for only 1 file
string CodeGenerator::generate() const {
    string code;

    // Ajouter l'implémentation des méthodes et constructeurs
    code += "\n// Methods and Constructors implementation:\n";
    code += implementation;
    code += "//\n\n";

    // Traiter le code global (ex: la fonction main)
    if (const auto block = dynamic_cast<BlockAST*>(ast.get())) {
        for (const auto& stmt : block->statements) {
            // On ne génère que ce qui n'est pas une définition de type (déjà fait)
            if (!dynamic_cast<StructDefinitionAST*>(stmt.get()) && !dynamic_cast<ExtendsStatementAST*>(stmt.get())) {
                code += stmt->code();
            }
        }
    }
    return code;
}

// TODO : generate constructor
// Should be called before generate method
string CodeGenerator::generateHeader() {
    string headerCode;
    map<string, vector<string>> structFields;      // map de struct -> code des champs de base
    map<string, set<string>> allStructFields;      // map de struct -> code de TOUS les champs (héritage inclus)
    map<string, set<string>> structMethods;        // map de struct -> prototype des méthodes
    map<string, vector<string>> structCtors;       // map de struct -> prototype des constructeurs

    // --- PASS 1: Collecter les définitions de struct (champs de base) ---
    if (const auto block = dynamic_cast<BlockAST*>(ast.get())) {
        for (const auto& stmt : block->statements) {
            if (const auto structDef = dynamic_cast<StructDefinitionAST*>(stmt.get())) {
                structFields.emplace(structDef->name, vector<string>());
                allStructFields.emplace(structDef->name, set<string>());
                for (const auto& field : structDef->fields) {
                    string fieldCode = field->code();
                    structFields[structDef->name].push_back(field->type->type + " " + field->name);
                    allStructFields[structDef->name].insert(fieldCode);
                }
            }
        }
    }

    // --- PASS 2: Gérer les extends (héritage, méthodes, constructeurs) ---
    // FIXME : Gérer l'ordre d'héritage serait plus robuste (e.g., analyse topologique)
    if (const auto block = dynamic_cast<BlockAST*>(ast.get())) {
        for (const auto& stmt : block->statements) {
            if (const auto ext = dynamic_cast<ExtendsStatementAST*>(stmt.get())) {
                // Assurer que les entrées existent
                if (!allStructFields.contains(ext->structName)) {
                    allStructFields.emplace(ext->structName, set<string>());
                }
                if (!structMethods.contains(ext->structName)) {
                    structMethods.emplace(ext->structName, set<string>());
                }
                if (!structCtors.contains(ext->structName)) {
                    structCtors.emplace(ext->structName, vector<string>());
                }

                // Cas de l'héritage
                if (!ext->parentStructName.empty()) {
                    for (const string& field : allStructFields[ext->parentStructName]) {
                        allStructFields[ext->structName].insert(field);
                    }
                    for (const string& method : structMethods[ext->parentStructName]) {
                        structMethods[ext->structName].insert(method);
                    }
                }

                // Add new members
                for (auto& member : ext->members) {
                    if (const auto var = dynamic_cast<VariableDeclarationAST*>(member.get())) {
                        allStructFields[ext->structName].insert(var->code());
                    } else if (const auto method = dynamic_cast<FunctionDefinitionAST*>(member.get())) {
                        // C'est une méthode
                        string proto = method->code(true);
                        //implementation += proto;
                        structMethods[ext->structName].insert(proto);
                    } else if (const auto ctor = dynamic_cast<ConstructorDefinitionAST*>(member.get())) {
                        // C'est un constructeur défini par l'utilisateur
                        // Il faut lui passer le nom de la struct !
                        ctor->structName = ext->structName;
                        string proto = ctor->code();
                        //implementation += proto;
                        structCtors[ext->structName].push_back(proto);
                    }
                }
            }
        }
    }

    // --- PASS 3: Generate structs definitions ---
    for (const auto& [name, fields] : allStructFields) {
        headerCode += "typedef struct {\n";
        for (const string& field : fields) {
            headerCode += '\t' + field + ";\n";
        }
        headerCode += "} " + name + ";\n\n";
    }

    // --- PASS 4: Générer les prototypes (constructeurs et méthodes) ---
    headerCode += "\n// Constructor prototypes\n";
    for (const auto& [name, protos] : structCtors) {
        // S'il y a des constructeurs customs, on les ajoute
        if (!protos.empty()) {
            for (const string& proto : protos) {
                headerCode += proto.substr(0, proto.find('{')) + ";" + "\n";
            }
        } else if (structFields.contains(name)) {
            // No constructor, fallback to default
            string signatureName = "fun_" + name;

            string paramsList = "";
            string ctorBody = " {\n\t" + name + "* self = alloc(sizeof(" + name + "));\n";

            const auto& fields = structFields.at(name);
            for (size_t i = 0; i < fields.size(); ++i) {
                const string& fullFieldDecl = fields[i];

                // On doit extraire le type et le nom du champ
                size_t spacePos = fullFieldDecl.find(' ');
                if (spacePos == string::npos) continue; // Sécurité, ne devrait pas arriver
                string fieldType = fullFieldDecl.substr(0, spacePos); // Ex: "int"
                string fieldName = fullFieldDecl.substr(spacePos + 1); // Ex: "x"

                // On ajoute le type au nom de la signature
                signatureName += "_" + fieldType; // Ex: "Point_new_int"

                // On ajoute la déclaration complète du paramètre à la liste
                paramsList += fullFieldDecl;

                // On ajoute l'assignation au corps de la fonction
                ctorBody += "\tself->" + fieldName + " = " + fieldName + ";\n";

                // Ajoute la virgule si ce n'est pas le dernier paramètre
                if (i < fields.size() - 1) {
                    paramsList += ", ";
                }
            }

            // 4. On assemble le tout pour former le prototype et l'implémentation
            string returnType = name + "* ";
            string proto = returnType + signatureName + "(" + paramsList + ");";
            string ctorImpl = returnType + signatureName + "(" + paramsList + ")";

            ctorBody += "\treturn self;\n}\n";

            headerCode += proto + "\n";
            implementation += ctorImpl + ctorBody;
        }
    }

    headerCode += "\n// Method prototypes\n";
    for (const auto& [name, protos] : structMethods) {
        for (const string& proto : protos) {
            string tmp = proto;
            tmp.insert(tmp.find("fun"), name + "_");
            tmp.insert(tmp.find("* self"), name);
            headerCode += tmp.substr(0, tmp.find('{')) + ";";

            implementation += tmp + '\n';
            /*size_t pos = implementation.find(tmp);
            if (pos != std::string::npos) {
                implementation.replace(pos, tmp.length(), tmp);
            }*/
        }
        headerCode += "\n";
    }

    return headerCode;
}
