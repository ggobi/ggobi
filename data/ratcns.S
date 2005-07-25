# Without R
# explore variables pairwise
# brush each of the four main functional groups (automated brushing works)
# bring up parcoords plot
# use shadow brushing to look at each group one at a time.

g = ggobi ("/home/dfs/ggobi/ggobi/data/ratcns.xml")

#
# Look at the functional groups one at a time by hiding
# all the cases not in the selected group.
#

ratcns <- ggobi.getData()
Func.Class.1 <- ratcns[,10]
Func.Class.2 <- ratcns[,11]
showClass <- function (n, .gobi=getDefaultGGobi()) {
   N <- length(Func.Class.2)
   tmp <- logical(N)
   if (n != -1) {
     tmp[Func.Class.2 != n] <- T
   }
   setHiddenCases.ggobi(tmp)
}

#
# cycle through classes
#
for (i in 1:14) {
 showClass(i)
 system("sleep 1")
}
showClass (-1)

#
# Don't forget that you want to cluster without the
# last variables.  And you want to take the variable Cluster
# out of the data.
#
dst <- dist(ratcns[,1:9])
hc <- hclust(dst, "w")
x11()
plot (hc)
cvar <- cutree (hc, 5)

addVariable.ggobi (cvar, "cluster")
showClust <- function (n, .gobi=getDefaultGGobi()) {
   N <- length(cvar)
   tmp <- logical(N)
   tmp[cvar != n] <- T
   setHiddenCases.ggobi(tmp)
}

#
# cycle through clusters n times
#
n = 3
for (k in 1:n) {
 for (i in 1:5) {
  showClust(i)
  system("sleep 2")
 }
}
showClass (-1)


#
# identification
#
idfunc <- function (i, display) {
  if (i >= 0) {
    k <- i+1
    print(k)
    print(ratcns[k,1:9])
  }
}
g$setIdentifyHandler(idfunc)
g$setMode("Identify")


# principal components
pc <- prcomp(ratcns[,1:9], center=T, scale=F)
for (i in 1:3) {
  name = paste("PC",i)
  addVariable.ggobi (as.numeric(pc$x[,i]), name)
}


