

class GGobiPythonPlugin:
    def __init__(self):
        print "In constructor"

    def onClose(self):
        print "[onClose]"
        return 1

    def onUpdateDisplay(self):
        print "[onUpdateDisplay]"        
        return 1    


