/*
Trabalho 2 - Software Básico - Tradutor
Vinícius Caixeta de Souza
180132199
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// struct que define quais serão os rótulos utilizados no arquivo .s
struct rotulo{
    string nome;
    int endereco;
    string dadoOuEnd;
};

int main(int argc, char *argv[]){
    int i;
    string arqObj, objeto, numeroObj;

    // Obtem arquivo .obj e cria arquivo .s
    arqObj = argv[1];
    ifstream arquivoObjeto(arqObj);
    arqObj.erase(arqObj.length() - 4);
    arqObj = arqObj + ".s";
    ofstream arquivoAsm(arqObj);

    /*====== Criação de rótulos ======*/

    vector<rotulo> rotulos;
    int quantOperando; // Armazena quatidade de operandos da instrução sendo lida
    bool instrucao = true, novaInst = false/*S_INPUT e S_OUTPUT*/;
    string dadoOuEnd;

    // Contagem de rótulos relacionados a dados
    int rotuloR = 1;
    // Contagem de rótulos relacionados a endereços
    int rotuloL = 1;

    getline(arquivoObjeto, objeto);
    istringstream rotulacao(objeto);

    while(getline(rotulacao, numeroObj, ' ')){
        // Seção de dados
        if(instrucao == false){
            for(i = 0; i < rotulos.size(); i++){
                if(rotulos[i].endereco == stoi(numeroObj)){
                    quantOperando--;
                    if(quantOperando == 0){
                        instrucao = true;
                    }
                    if(novaInst == true){
                        // Segundo operando de S_INPUT e S_OUTPUT é irrelevante
                        getline(rotulacao, numeroObj, ' ');
                        instrucao = true;
                        novaInst = false;
                    }
                    goto cont;
                }
            }

            if(novaInst == true){
                // Segundo operando de S_INPUT e S_OUTPUT é irrelevante
                getline(rotulacao, numeroObj, ' ');
                instrucao = true;
                novaInst = false;
            }

            if(dadoOuEnd == "dado"){
                rotulos.push_back({"R" + to_string(rotuloR), stoi(numeroObj), dadoOuEnd});
                rotuloR++;
                quantOperando--;
                if(quantOperando == 0){
                    instrucao = true;
                    continue;
                }
            }
            if(dadoOuEnd == "end"){
                rotulos.push_back({"L" + to_string(rotuloL), stoi(numeroObj), dadoOuEnd});
                rotuloL++;
                quantOperando--;
                if(quantOperando == 0){
                    instrucao = true;
                    continue;
                }
            }
            
        } // Seção de instruções
        else{
            // Coloca instrução como false pois na próxima interação do while será um operando
            instrucao = false;

            // ADD, SUB, MUL, DIV, LOAD, STORE, INPUT, OUTPUT
            if(numeroObj == "1" || numeroObj == "2" || numeroObj == "3" || numeroObj == "4" || numeroObj == "10" || numeroObj == "11" ||
            numeroObj == "12" || numeroObj == "13"){
                quantOperando = 1;
                dadoOuEnd = "dado";
                continue;
            }
            // JMP, JMPN, JMPP, JMPZ
            if(numeroObj == "5" || numeroObj == "6" || numeroObj == "7" || numeroObj == "8"){
                quantOperando = 1;
                dadoOuEnd = "end";
                continue;
            }
            // COPY
            if(numeroObj == "9"){
                quantOperando = 2;
                dadoOuEnd = "dado";
                continue;
            }
            // STOP
            if(numeroObj == "14"){ break; }
            // S_INPUT, S_OUTPUT
            if(numeroObj == "15" || numeroObj == "16"){
                novaInst = true;
                quantOperando = 2;
                dadoOuEnd = "dado";
                continue;
            }
        }

        cont:;
    }

    /*====== Criação de código NASM ======*/
    /*==== Seção Texto ====*/

    bool fimSecaoText = false;
    int contPosicao = 0;
    string operando, operandoAtual;

    // Função de output no arquivo de saída
    arquivoAsm << "output:" << endl;
    arquivoAsm << "\tenter 0, 0" << endl;
    arquivoAsm << "\tmov ecx, [ebp + 8]" << endl;
    arquivoAsm << "\tmov eax, 4" << endl;
    arquivoAsm << "\tmov ebx, 1" << endl;
    arquivoAsm << "\tmov edx, 4" << endl;
    arquivoAsm << "\tint 80h" << endl;
    arquivoAsm << "\tmov eax, 4" << endl;
    arquivoAsm << "\tmov ebx, 1" << endl;
    arquivoAsm << "\tmov ecx, nwln" << endl;
    arquivoAsm << "\tmov edx, NWLNSIZE" << endl;
    arquivoAsm << "\tint 80h" << endl;
    arquivoAsm << "\tmov eax, 4" << endl;
    arquivoAsm << "\tleave" << endl;
    arquivoAsm << "\tret 4" << endl;

    arquivoAsm << "\nsection .text\nglobal _start\n_start:\n";

    istringstream criacaoAsm(objeto);

    while(fimSecaoText == false && getline(criacaoAsm, numeroObj, ' ')){
        // Rótulo de endereço
        for(i = 0; i < rotulos.size(); i++){
            if(contPosicao == rotulos[i].endereco && rotulos[i].dadoOuEnd == "end"){
                arquivoAsm << rotulos[i].nome << ':';
            }
        }

        // Operando da instrução
        if(stoi(numeroObj) != 14/*STOP*/){
            getline(criacaoAsm, operando, ' ');
            for(i = 0; i < rotulos.size(); i++){
                if(stoi(operando) == rotulos[i].endereco){ operandoAtual = rotulos[i].nome; }
            }
        }

        contPosicao += 2;

        switch(stoi(numeroObj)){
            // ADD
            case 1:
                arquivoAsm << "\tadd eax, " << operandoAtual << endl;
                break;
            // SUB
            case 2:
                arquivoAsm << "\tsub eax, " << operandoAtual << endl;
                break;
            // MUL
            case 3:
                arquivoAsm << "\timul dword [" << operandoAtual << "]\n";
                arquivoAsm << "\tpush eax" << endl;
                arquivoAsm << "\tcall TRATAR_OVERFLOW\n";
                arquivoAsm << "\tpop eax" << endl;
                break;
            // DIV
            case 4:
                arquivoAsm << "\tcdq\n";
                arquivoAsm << "\tidiv dword [" << operandoAtual << "]\n";
                break;
            // JMP
            case 5:
                arquivoAsm << "\tjmp " << operandoAtual << endl;
                break;
            // JMPN
            case 6:
                arquivoAsm << "\tjl " << operandoAtual << endl;
                break;
            // JMPP
            case 7:
                arquivoAsm << "\tjg " << operandoAtual << endl;
                break;
            // JMPZ
            case 8:
                arquivoAsm << "\tje " << operandoAtual << endl;
                break;
            // COPY
            case 9:
                arquivoAsm << "\tpush eax\n";
                arquivoAsm << "\tmov eax, [" << operandoAtual << "]\n";
                getline(criacaoAsm, operando, ' ');
                for(i = 0; i < rotulos.size(); i++){
                    if(stoi(operando) == rotulos[i].endereco){ operandoAtual = rotulos[i].nome; }
                }
                arquivoAsm << "\tmov [" << operandoAtual << "], eax\n";
                arquivoAsm << "\tpop eax\n";
                contPosicao++;
                break;
            // LOAD
            case 10:
                arquivoAsm << "\tmov eax, [" << operandoAtual << "]\n";
                break;
            // STORE
            case 11:
                arquivoAsm << "\tmov [" << operandoAtual << "], eax\n";
                break;
            // INPUT - NÃO IMPLEMENTADA
            case 12:
                break;
            // OUTPUT
            case 13:
                arquivoAsm << "\tpush eax" << endl;
                arquivoAsm << "\tmov eax, [" << operandoAtual << ']' << endl;
                arquivoAsm << "\tadd eax, 0x30" << endl;
                arquivoAsm << "\tmov [" << operandoAtual << "], eax" << endl;
                arquivoAsm << "\tpush dword " << operandoAtual << endl;
                arquivoAsm << "\tcall output" << endl;
                arquivoAsm << "\tadd eax, 0x30" << endl;
                arquivoAsm << "\tmov [bytesEscritos], eax" << endl;
                arquivoAsm << "\tmov eax, 4" << endl;
                arquivoAsm << "\tmov ebx, 1" << endl;
                arquivoAsm << "\tmov ecx, msg1" << endl;
                arquivoAsm << "\tmov edx, MSG1SIZE" << endl;
                arquivoAsm << "\tint 80h" << endl;
                arquivoAsm << "\tmov eax, 4" << endl;
                arquivoAsm << "\tmov ebx, 1" << endl;
                arquivoAsm << "\tmov ecx, bytesEscritos" << endl;
                arquivoAsm << "\tmov edx, 4" << endl;
                arquivoAsm << "\tint 80h" << endl;
                arquivoAsm << "\tmov eax, 4" << endl;
                arquivoAsm << "\tmov ebx, 1" << endl;
                arquivoAsm << "\tmov ecx, msg2" << endl;
                arquivoAsm << "\tmov edx, MSG2SIZE" << endl;
                arquivoAsm << "\tint 80h" << endl;
                arquivoAsm << "\tmov eax, 4" << endl;
                arquivoAsm << "\tmov ebx, 1" << endl;
                arquivoAsm << "\tmov ecx, nwln" << endl;
                arquivoAsm << "\tmov edx, NWLNSIZE" << endl;
                arquivoAsm << "\tint 80h" << endl;
                arquivoAsm << "\tpop eax" << endl;
                break;
            // STOP
            case 14:
                arquivoAsm << "\tmov eax, 1\n\tmov ebx, 0\n\tint 80h\n";
                contPosicao--;
                fimSecaoText = true;
                break;
            // S_INPUT - NÃO IMPLEMENTADA
            case 15:
                getline(criacaoAsm, operando, ' ');
                break;
            // S_OUTPUT - NÃO IMPLEMENTADA
            case 16:
                getline(criacaoAsm, operando, ' ');
                break;
        }
    }

    // Função responsável por tratar overflow na multiplicação
    arquivoAsm << "\nTRATAR_OVERFLOW:\n";
    arquivoAsm << "\tpush edx\n";
    arquivoAsm << "\tor edx, 0x00000000\n";
    arquivoAsm << "\tjne T1\n";
    arquivoAsm << "\tpop edx\n";
    arquivoAsm << "\tret\n";
    arquivoAsm << "T1:\n";
    arquivoAsm << "\tpop edx\n";
    arquivoAsm << "\tsub edx, 0xffffffff\n";
    arquivoAsm << "\tjne OVERFLOW\n";
    arquivoAsm << "\tret\n";
    arquivoAsm << "OVERFLOW:\n";
    arquivoAsm << "\tmov eax, 4\n";
    arquivoAsm << "\tmov ebx, 1\n";
    arquivoAsm << "\tmov ecx, ovflw\n";
    arquivoAsm << "\tmov edx, OVFLWSIZE\n";
    arquivoAsm << "\tint 80h\n";
    arquivoAsm << "\tmov eax, 4\n";
    arquivoAsm << "\tmov ebx, 1\n";
    arquivoAsm << "\tmov ecx, nwln\n";
    arquivoAsm << "\tmov edx, NWLNSIZE\n";
    arquivoAsm << "\tint 80h\n";
    arquivoAsm << "\tmov eax, 1\n\tmov ebx, 0\n\tint 80h\n";

    /*==== Seção dados (data/bss) ====*/

    string secaoInicial;

    arquivoAsm << endl;

    getline(criacaoAsm, numeroObj, ' ');

    // Cria seção de dados assumindo que todos os dados inicializados e não inicializados estão
    // separados corretamente
    if(stoi(numeroObj) == 0){
        secaoInicial = "dadoNaoInicializado";
        arquivoAsm << "section .bss" << endl;
        arquivoAsm << "bytesEscritos resd 1" << endl;

        for(i = 0; i < rotulos.size(); i++){
            if(contPosicao == rotulos[i].endereco && rotulos[i].dadoOuEnd == "dado"){
                arquivoAsm << rotulos[i].nome << " resd 1" << endl;
            }
        }
    }
    else{
        secaoInicial = "dadoInicializado";
        arquivoAsm << "section .data" << endl;
        arquivoAsm << "ovflw db \"overflow\", 0" << endl;
        arquivoAsm << "OVFLWSIZE EQU $-ovflw" << endl;
        arquivoAsm << "nwln db 0ah" << endl;
        arquivoAsm << "NWLNSIZE EQU $-nwln" << endl;
        arquivoAsm << "msg1 db \"Foram escritos \", 0" << endl;
        arquivoAsm << "MSG1SIZE EQU $-msg1" << endl;
        arquivoAsm << "msg2 db \" bytes\", 0" << endl;
        arquivoAsm << "MSG2SIZE EQU $-msg2" << endl;

        for(i = 0; i < rotulos.size(); i++){
            if(contPosicao == rotulos[i].endereco && rotulos[i].dadoOuEnd == "dado"){
                arquivoAsm << rotulos[i].nome << " dd " << numeroObj << endl;
            }
        }
    }

    contPosicao++;

    while(getline(criacaoAsm, numeroObj, ' ')){

        if(secaoInicial == "dadoInicializado" && stoi(numeroObj) == 0){
            arquivoAsm << endl << "section .bss" << endl;
            arquivoAsm << "bytesEscritos resd 1" << endl;
            secaoInicial = "dadoNaoInicializado";
        }
        if(secaoInicial == "dadoNaoInicializado" && stoi(numeroObj) != 0){
            arquivoAsm << endl << "section .data" << endl;
            arquivoAsm << "ovflw db \"overflow\", 0" << endl;
            arquivoAsm << "OVFLWSIZE EQU $-ovflw" << endl;
            arquivoAsm << "nwln db 0ah" << endl;
            arquivoAsm << "NWLNSIZE EQU $-nwln" << endl;
            arquivoAsm << "msg1 db \"Foram escritos \", 0" << endl;
            arquivoAsm << "MSG1SIZE EQU $-msg1" << endl;
            arquivoAsm << "msg2 db \" bytes\", 0" << endl;
            arquivoAsm << "MSG2SIZE EQU $-msg2" << endl;
            secaoInicial = "dadoInicializado";
        }

        if(stoi(numeroObj) == 0){
            for(i = 0; i < rotulos.size(); i++){
                if(contPosicao == rotulos[i].endereco && rotulos[i].dadoOuEnd == "dado"){
                    arquivoAsm << rotulos[i].nome << " resd 1" << endl;
                }
            }
        }
        else{
            for(i = 0; i < rotulos.size(); i++){
                if(contPosicao == rotulos[i].endereco && rotulos[i].dadoOuEnd == "dado"){
                    arquivoAsm << rotulos[i].nome << " dd " << numeroObj << endl;
                }
            }
        }

        contPosicao++;
    }

    arquivoObjeto.close();
    arquivoAsm.close();

    return 0;
}