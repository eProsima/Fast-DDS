#!/usr/bin/python


import matplotlib.pyplot as plt
import numpy as np

rtidata=np.loadtxt("res_rti_latency.txt",delimiter=',')
eprodata=np.loadtxt("res_epro_latency.txt",delimiter=',')

label=['Stdev','Mean','Min','50% ','90%','99%','99.99%','Max']
dim = len(eprodata[:,0])
w = 0.25
xpos = np.arange(len(eprodata[:,0]))+1
xpos2=xpos+w
for col in range(1,9):
	plt.figure()
	plt.bar(xpos2,eprodata[:,col]/2,w,color='b',label='eProsima')
	plt.bar(xpos,rtidata[:,col]/2,w,color='yellow',label='RTI-DDS')
	plt.title(r'$Mean\ Latency$')
	plt.xlim(0.5,10)
	plt.xlabel(r'$Payload\ size\ (bytes)$',fontsize=15)
	plt.ylabel(r'$Time\ (\mu\ s)$',fontsize=15)
	plt.xticks(xpos2,eprodata[:,0].astype(int))
	plt.legend(loc='upper left')
	plt.savefig(label[col-1]+'.png', bbox_inches='tight')
#plt.show()

	
