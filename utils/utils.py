def Tuple2String(tup):
    string = ''
    for index in range(0, len(tup)):
        string += str(tup[index])
        string += ','
    return string


def String2Tuple(string):
    Tuple = []
    strlen = len(string)
    tmpstring = ''
    for index in range(0, strlen):
        if(string[index] == ','):
            if(tmpstring != ''):
                Tuple.append(int(tmpstring))
            tmpstring = ''
        else:
            tmpstring += string[index]
    Tuple = tuple(Tuple)
    return Tuple
