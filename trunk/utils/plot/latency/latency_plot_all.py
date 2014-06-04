#!/usr/bin/python


import matplotlib.pyplot as plt
import numpy as np

rtidata=np.loadtxt("rti_latency.txt",delimiter=',')
eprodata=np.loadtxt("epro_latency.txt",delimiter=',')

label=['Stdev','Mean','Min','50% ','90%','99%','99.99%','Max']
dim = len(eprodata[:,0])
w = 0.25
xpos = np.arange(len(eprodata[:,0]))+1
xposrti=xpos+w
for col in range(1,9):
	plt.figure()
	plt.bar(xpos,eprodata[:,col],w,color='b',label='eProsima')
	plt.bar(xposrti,rtidata[:,col],w,color='yellow',label='RTI-DDS')
	plt.title(label[col-1])
	plt.xlim(0.5,10)
	plt.xlabel(r'$Payload\ size\ (bytes)$',fontsize=15)
	plt.ylabel(r'$Time\ (\mu\ s)$',fontsize=15)
	plt.xticks(xposrti,eprodata[:,0].astype(int))
	plt.legend(loc='upper left')
	plt.savefig(label[col-1]+'.png', bbox_inches='tight')
#plt.show()

	
