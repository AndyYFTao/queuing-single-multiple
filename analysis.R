install.packages("plotly")
library(plotly)
setwd("~/Desktop/YEAR 4/summer/IT course timetable/project/simulation_final")

sraw <- read.delim("simulation_data_s.txt",skip=1,header=TRUE)
s <- data.matrix(sraw)
mraw <- read.delim("simulation_data_m.txt",skip=1,header=TRUE)
m <- data.matrix(mraw)

average_waiting_time <- c(s)
p <- plot_ly(y = ~average_waiting_time, type = "box",name = "single line") %>%
  add_trace(y = ~c(m),name="multiple lines")%>%
  layout(title = "Boxplot (4 windows, 70 clients)")
p

sefficient <- vector()
for (i in 1:100){
  if (s[i] < m[i]){
    sefficient[i] <- 1
  }else{
    sefficient[i] <- 0
  }
}
sbetter <- sum(sefficient)/100
mbetter <- 1-sbetter
