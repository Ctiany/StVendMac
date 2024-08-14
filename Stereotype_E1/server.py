import socket
import hmac
import hashlib
import codecs

#Please fill in the IP addressess of accepted clients (aka the board) on line 13 and 42.
#The IP information will be printed in the Serial output in the Arduino IDE software,
#when the board boots up while connected to the computer.

#The exact code for doing that is on line 405 in the board code.

genuine_data = {
    "ACCEPTED_CLIENTS" : {
        "commodityNumber" : 2,
        "commodityPrices" : [10, 7, 12],
        "genuineIDSize" : 2,
        "genuineID" : ["a756a7c0291836c5fd8192f3717004fd903d184297f734e882d3b9b732ee58fe", "974ed452fd8dcb41ac4f694c53555e7692f50f46b9597e2b4a80fc37cec189ea", "eca1660f0a3087b680b9a87892e9e139c7c389100b4554c8080e619e0f683c8c"],
        "comPins" : [14, 27, 26],
        "balances" : {
            "a756a7c0291836c5fd8192f3717004fd903d184297f734e882d3b9b732ee58fe" : 14,
            "974ed452fd8dcb41ac4f694c53555e7692f50f46b9597e2b4a80fc37cec189ea" : 1454,
            "eca1660f0a3087b680b9a87892e9e139c7c389100b4554c8080e619e0f683c8c" : 1514
        }
    },
    #This is for testing.
    "127.0.0.1" :{
        "commodityNumber" : 2,
        "commodityPrices" : [10, 7, 12],
        "genuineIDSize" : 2,
        "genuineID" : ["a756a7c0291836c5fd8192f3717004fd903d184297f734e882d3b9b732ee58fe", "974ed452fd8dcb41ac4f694c53555e7692f50f46b9597e2b4a80fc37cec189ea", "eca1660f0a3087b680b9a87892e9e139c7c389100b4554c8080e619e0f683c8c"],
        "comPins" : [14, 27, 26],
        "balances" : {
            "a756a7c0291836c5fd8192f3717004fd903d184297f734e882d3b9b732ee58fe" : 14,
            "974ed452fd8dcb41ac4f694c53555e7692f50f46b9597e2b4a80fc37cec189ea" : 1454,
            "eca1660f0a3087b680b9a87892e9e139c7c389100b4554c8080e619e0f683c8c" : 1514
        }
    }
}

#Data stored, should be teh same as on the machine.
sugar = {
    "ACCEPTED_CLIENT" : 0,
    "127.0.0.1" : 0
}
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('', 5556)
sock.bind(server_address)
sock.listen(1)
while True:
    connection, client_address = sock.accept()
    print("Connection from ", str(client_address[0]), ": ", sep = '', end = '')
    if str(client_address[0]) in genuine_data:
        sugar[str(client_address[0])] += 1
        mssg = connection.recv(1024)
        print(mssg)
        
        this_data = genuine_data[str(client_address[0])]
        commodityNumber = this_data["commodityNumber"]
        genuineIDSize = this_data["genuineIDSize"]
        genuineID = this_data["genuineID"]
        commodityPrices = this_data["commodityPrices"]
        comPins = this_data["comPins"] 
        temp_str = ""
        temp_str += str(commodityNumber)          
        for i in commodityPrices:
            temp_str += str(i)
        for i in comPins:
            temp_str += str(i)
        KEY = 'thisIsAMachine112@@!!ThisIsAMachine>>!'
        temp_str += str(sugar[str(client_address[0])])
        print("INITIAL temp_str: " + temp_str)
        if "VERIFY" in codecs.decode(mssg):
            h = hmac.new(codecs.encode(KEY), codecs.encode(temp_str), hashlib.sha256).hexdigest()
            tempdt = str(genuine_data[str(client_address[0])]["genuineIDSize"])
            while len(tempdt) < 5:
                tempdt = '0' + tempdt
            h += tempdt
            
            for i in genuineID:
                h += i
                tempdt = str(genuine_data[str(client_address[0])]["balances"][i])
                while len(tempdt) < 10:
                    tempdt = '0' + tempdt
                h += tempdt
            connection.sendall(codecs.encode(h))
        if "UPDATE" in codecs.decode(mssg):
            if len(codecs.decode(mssg)) < 143:
                connection.close()
                continue
            verDigit = codecs.decode(mssg)[6 : 70]
            updateTarget = codecs.decode(mssg)[70 : 134]
            updateVolumn = codecs.decode(mssg)[134 : 144]
            print("VER DIGIT: " + verDigit + "; TARGET: " + updateTarget + "; VOLUMN: "+ updateVolumn)
            print("TEMP: " + temp_str + "\nSUGAR: " + str(sugar))
            print("LOCAL VER: " + hmac.new(codecs.encode(KEY), codecs.encode(temp_str), hashlib.sha256).hexdigest())
            if not hmac.new(codecs.encode(KEY), codecs.encode(temp_str), hashlib.sha256).hexdigest() == verDigit:
                connection.sendall(codecs.encode("AUTHFAIL"))
                connection.close()
                continue
            if not updateTarget in genuineID:
                connection.sendall(codecs.encode("NOTFOUND"))
                connection.close()
                continue
            if not updateVolumn.isdigit():
                connection.sendall(codecs.encode("NOTDIGIT"))
                connection.close()
                continue
            genuine_data[str(client_address[0])]["balances"][updateTarget] -= int(updateVolumn)
            print("UPDATED:", genuine_data[str(client_address[0])]["balances"][updateTarget])
            connection.sendall(codecs.encode("OPERDONE"))
        connection.close()