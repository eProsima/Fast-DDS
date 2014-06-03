#!/usr/bin/python


import matplotlib.pyplot as plt
import numpy as np

rtidata=np.loadtxt("rti_latency.txt",delimiter=',')
eprodata=np.loadtxt("epro_latency.txt",delimiter=',')

label=['Mean','Min','Max','Stdev','50% ','90%','99%','99.99%']
dim = len(eprodata[:,0])
w = 0.25
xpos = np.arange(len(eprodata[:,0]))+1
xposrti=xpos+w
#for col in range(2,10)
plt.figure()
plt.bar(xpos,eprodata[:,1],w,color='b',label='eProsima')
plt.bar(xposrti,rtidata[:,2],w,color='yellow',label='RTI-DDS')
plt.title(label[0])
plt.xlim(0.5,10)
plt.xlabel(r'$Payload\ size\ (bytes)$',fontsize=15)
plt.ylabel(r'$Time\ (\mu\ s)$',fontsize=15)
plt.xticks(xposrti,eprodata[:,0].astype(int))
plt.legend(loc='upper left')
plt.show()

	
