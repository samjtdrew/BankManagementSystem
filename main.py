import socket
import logging
import pyodbc
import random
import datetime
from _socket import SHUT_RDWR


def accessParameters(data, searchfor):
    for value in data:
        if searchfor in value:
            parameters = value[(len(searchfor) + 2):].split(":")
            return parameters
    return "ERR>>MISSING"


def genHexKey(username, password):
    denary = random.randint(21267647932558653966460912964485513216,
                            340282366920938463463374607431768211455)
    hexadecimal = hex(denary)
    hexa = str(hexadecimal)
    hexa = list(hexa[2:])
    UCSl = []
    count = 0
    for digit in hexa:
        count += 1
        UCSl.append(digit)
        if (count % 4) == 0 and count < 30:
            UCSl.append("-")

    key = ''.join(UCSl)
    sessionKeys.append(key + "," + username + "," + password)
    return key


def delHexKey(hexKey):
    for index in range(0, len(sessionKeys)):
        if hexKey in sessionKeys[index]:
            sessionKeys[index] = ""
            logging.info(f"Session Key: {hexKey} Deleted")
            return "Session Terminated"
    return "Error[001]"


def translate(data, sendsock):
    # string format:  DATA1|DATA2|DATA3<>PARAM1:PARAM2:PARAM3:PARAM4|DATA4<>PARAM1:PARAM2
    # DATA1 should be the temporary hex code (in format HEXKEY<>'insert hexkey'), or 'NULL' if not logged in
    # splits into data chunks
    chunk = data.split("|")
    if "TERMINATE" in data:
        # check user is logged in
        inkey = accessParameters(chunk, "HEXKEY")
        if inkey[0] in str(sessionKeys):
            logging.warning("Network Shutdown Request Received")
            logging.warning("Attempting Safe Network Termination")
            clientsocket.close()
            sock.shutdown(SHUT_RDWR)
            logging.critical("Network Terminated")
            logging.shutdown()
            exit()
    if "LOGIN" in data:
        # send the array through sequence to index parameters
        response = accessParameters(chunk, "LOGIN")
        if response == "ERR>>MISSING":
            logging.warning(f"Invalid Byte Query Received [{data}]")
        else:
            # expected parameters for LOGIN >> Username:Password
            cursor = conn.cursor()
            cursor.execute("SELECT Username FROM BankingAuthentication.dbo.UserCredentials")
            usernames = cursor.fetchall()
            for users in usernames:
                if str(users).replace("('", "").replace("', )", "").strip() == response[0]:
                    cursor.execute("SELECT Password FROM BankingAuthentication.dbo.UserCredentials")
                    passwords = cursor.fetchall()
                    for passes in passwords:
                        if str(passes).replace("('", "").replace("', )", "").strip() == response[1]:
                            hexKey = genHexKey(response[0], response[1])
                            cursor.execute("SELECT UserRole FROM BankingAuthentication.dbo.UserCredentials WHERE "
                                           "Username = '" + response[0] + "'")
                            userClass = cursor.fetchall()
                            hexKey += "|" + str(userClass[0]).replace("('", "").replace("', )", "").strip() + '\0'
                            totalsent = 0
                            while totalsent < len(hexKey):
                                sent = sendsock.send(hexKey.encode('utf-8'))
                                if sent == 0:
                                    logging.warning("Unclean Socket Termination")
                                    delHexKey(hexKey)
                                totalsent += sent
                            logging.info(f'Hex Key Transmitted To Client: {hexKey[:39]}')
            cursor.close()
    if "LOGOUT" in data:
        hexKey = accessParameters(chunk, "HEXKEY")
        reply = delHexKey(hexKey[0]) + '\0'
        totalsent = 0
        while totalsent < len(reply):
            sent = sendsock.send(reply.encode('utf-8'))
            if sent == 0:
                logging.warning("Unclean Socket Termination")
            totalsent += sent
    if "SIGNUP" in data:
        # Expected Input 'HEXKEY<>[insert hexkey]|SIGNUP<>[newusername]:[newpassword]:[newuserclass]'
        # check user is logged in
        inkey = accessParameters(chunk, "HEXKEY")
        if inkey[0] in str(sessionKeys):
            # take users params
            response = accessParameters(chunk, "SIGNUP")
            cursor = conn.cursor()
            denary = random.randint(21267647932558653966460912964485513216, 340282366920938463463374607431768211455)
            hexadecimal = hex(denary)
            hexa = str(hexadecimal)
            hexa = list(hexa[2:])
            UCSl = []
            count = 0
            for digit in hexa:
                count += 1
                UCSl.append(digit)
                if (count % 4) == 0 and count < 30:
                    UCSl.append("-")
            UCS = ''.join(UCSl)
            # add new user credentials
            cursor.execute("INSERT INTO BankingAuthentication.dbo.UserCredentials (UniqueUserID, Username, Password,"
                           " UserRole, UniqueConnectionString) VALUES ('00000000', '" + response[0] + "', '" +
                           response[1] + "', '" + response[2] + "', '" + UCS + "')")
            # create new user table
            cursor.execute("CREATE TABLE BankingData." + response[2] + ".[" + response[0] + "] (Balance nchar(25))")
            cursor.execute("INSERT INTO BankingData." + response[2] + ".[" + response[0] + "] (Balance)"
                                                                                           "VALUES ('0')")
            conn.commit()
            logging.info(f"New User Created: {response[0]}")
            cursor.close()
    if "FETCHBAL" in data:
        # Expected Input 'HEXKEY<>[insert hexkey]|FETCHBAL<>[username]:[userclass]'
        # check user is logged in
        inkey = accessParameters(chunk, "HEXKEY")
        if inkey[0] in str(sessionKeys):
            # take users params
            response = accessParameters(chunk, "FETCHBAL")
            cursor = conn.cursor()
            cursor.execute("SELECT Balance FROM BankingData." + response[1] + ".[" + response[0] + "]")
            try:
                balance = str(cursor.fetchall()[0]).replace("('", "").replace("', )", "").strip() + '\0'
            except IndexError:
                balance = "0" + '\0'
            totalsent = 0
            while totalsent < len(balance):
                sent = sendsock.send(balance.encode('utf-8'))
                if sent == 0:
                    logging.warning("Unclean Socket Termination")
                totalsent += sent
            logging.info(f'Balance Transmitted To Client: {balance}')
            cursor.close()
    if "UPDATEBAL" in data:
        # Expected Input 'HEXKEY<>[insert hexkey]|FETCHBAL<>[username]:[userclass]:[newbalance]'
        # check user is logged in
        inkey = accessParameters(chunk, "HEXKEY")
        if inkey[0] in str(sessionKeys):
            # take users params
            response = accessParameters(chunk, "UPDATEBAL")
            cursor = conn.cursor()
            cursor.execute("UPDATE BankingData." + response[1] + ".[" + response[0] + "] SET Balance = '" + response[2]
                           + "'")
            print("UPDATE BankingData." + response[1] + ".[" + response[0] + "] SET Balance = '" + response[2]
                  + "'")
            conn.commit()
            logging.info(f'Balance Of Client Updated To: {response[2]}')
            cursor.close()


# setup log
logging.basicConfig(filename='/usr/local/bin/connections.log', filemode='a', level=logging.DEBUG)
logging.info(f"-----Logging Started [{str(datetime.datetime.now().strftime('%A %d %b|%X'))}]-----")
# start websocket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('', 30003))
sock.listen(5)
alive = True

sessionKeys = []

conn = pyodbc.connect("FILEDSN=/usr/local/bin/ODBC.dsn")

while alive:
    # accept clientsocket
    clientsocket, clientaddr = sock.accept()
    logging.info(f'Connection From {clientaddr[0]}')
    incoming = clientsocket.recv(1024)
    if incoming == b'':
        logging.error('Unclean Socket Termination')
    translate(incoming.decode('utf-8'), clientsocket)
    clientsocket.close()
