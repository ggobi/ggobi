import GGobiPlugin

class SamplePlugin(GGobiPlugin.GGobiPythonPlugin):
    def __init__(self):
        print "In SamplePlugin constructor... Calling inherited method" 
        GGobiPlugin.GGobiPythonPlugin.__init__(self)

    def onUpdateDisplay(self):
        print "overloaded method for onUpdateDisplay"
        return 1
    
