#
# These functions are used to process
# Still need some work to get all the details right
# but produces basics.
#
#
# You will need the XML package for S (i.e. R or S-Plus)
# available from http://www.omegahat.org/XML
# or via the update.packages() function in R.
#
# An XSL-based mechanism may be available in the future.
#


getArgInfo <-
function(fileName = "../../Docs/commandArgs.xml")
{
  require(XML)
  args <- xmlRoot(xmlTreeParse(fileName))

  info <- list()
   xmlSApply(args, 
             function(x) {
               at <- xmlAttrs(x)
               name <- at[["name"]]
               if("alias" %in% names(at)) {
                  alias <- at[["alias"]]
                  if( alias %in% names(info)) {
                     info[[alias]]$aliases <<- c(info[[alias]]$aliases, name)
                  } 
                  return(FALSE)
               }

               desc <- x[["description"]]
               info[[name]] <<- list(description = desc, quickHelp = x[["quickHelp"]])
               if("novalue" %in% names(at))
                 info[[name]]$novalue <<- as.logical(at[["novalue"]])
               if("type" %in% names(at))
                 info[[name]]$type <<- at[["type"]]               
             })

  info
}

createCmdArgHelp <-
function(info)
{
 info <- info[sort(names(info))]

 out <- character()
 for(i in names(info)) {
   out[i] <- paste("\t", '"', flagString(info[[i]], i), '", "', asSimpleText(info[[i]]$quickHelp), '"', sep="")
   if(!is.null(info[[i]]$type) && info[[i]]$type == "both")
     out[length(out)+1] <- paste("\t", '"-', i, '", "', '"', sep="")
   if(!is.null(info[[i]]$aliases)) {
     for(j in info[[i]]$aliases) {
       out[length(out)+1] <- paste("\t", '"-', j, '", "', '"', sep="")
     }
   }
 }

 paste(out, collapse=",\n", sep="")
}

flagString <-
function(desc, name)
{
  if(is.null(desc$type))
     prefix <- "-"
   else
     prefix <- "--"

  paste(prefix, name, sep="")
}  


asSimpleText <-
function(node)
{
 if(is.null(node))
   return("")
 
 txt <- xmlSApply(node, function(x)
                    if(inherits(x, "XMLTextNode"))
                      xmlValue(x)
                    else
                      asSimpleText(x))


 paste(txt, collapse="", sep="")
}  
