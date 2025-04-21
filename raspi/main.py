import serial
import time

def conectar_arduino():
    try:
        return serial.Serial('/dev/ttyUSB0', 9600, timeout=2)
    except:
        print("Erro: não foi possível conectar ao Arduino.")
        return None

def enviar_comando(arduino, comando):
    arduino.write((comando + '\n').encode())
    time.sleep(0.5)
    while arduino.in_waiting:
        resposta = arduino.readline().decode().strip()
        print("Resposta:", resposta)

def menu():
    print("\n====== MENU ARDUINO ======")
    print("[1] Mover para frente")
    print("[2] Mover para trás")
    print("[3] Parar")
    print("[4] Medir distância")
    print("[5] Sair")
    print("==========================")

arduino = conectar_arduino()
if arduino:
    time.sleep(2)
    while True:
        menu()
        opcao = input("Escolha uma opção: ")

        if opcao == "1":
            enviar_comando(arduino, "FRENTE")
        elif opcao == "2":
            enviar_comando(arduino, "TRAS")
        elif opcao == "3":
            enviar_comando(arduino, "PARAR")
        elif opcao == "4":
            enviar_comando(arduino, "DISTANCIA")
        elif opcao == "5":
            print("Encerrando...")
            arduino.close()
            break
        else:
            print("Opção inválida.")
