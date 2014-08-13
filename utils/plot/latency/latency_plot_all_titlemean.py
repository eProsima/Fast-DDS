#!/usr/bin/python


import matplotlib.pyplot as plt
import numpy as np

rtidata=np.loadtxt("res_rti_latency_1_1.txt",delimiter=',')
eprodata=np.loadtxt("res_epr_latency_1_2.txt",delimiter=',')

label=['Stdev','Mean','Min','50% ','90%','99%','99.99%','Max']
dim = len(eprodata[:,0])
w = 0.25
xpos = np.arange(len(eprodata[:,0]))+1
xpos2=xpos+w
for col in range(2,3):
	#plt.figure()
	#plt.bar(xpos2,eprodata[:,col]/2,w,color='b',label='eProsima')
	#plt.bar(xpos,rtidata[:,col]/2,w,color='green',label='RTI-DDS')
	#plt.title(r'$Mean\ Latency$')
	#plt.xlim(0.5,11)
	#plt.xlabel(r'$Payload\ size\ (bytes)$',fontsize=15)
	#plt.ylabel(r'$Time\ (\mu\ s)$',fontsize=15)
	#plt.xticks(xpos2,eprodata[:,0].astype(int))
	#plt.legend(loc='upper left')
	#plt.savefig(label[col-1]+'.png', bbox_inches='tight')
	#LINE PLOT
	plt.figure()
	plt.plot(xpos,eprodata[:,col]/2,'-o',color='b',label='eProsima')
	plt.plot(xpos,rtidata[:,col]/2,'-o',color='green',label='RTI-DDS')
	plt.title(r'$Mean\ Latency$')
	plt.xlim(0.5,11)
	plt.xlabel(r'$Payload\ size\ (bytes)$',fontsize=15)
	plt.ylabel(r'$Time\ (\mu\ s)$',fontsize=15)
	plt.xticks(xpos,eprodata[:,0].astype(int))
	plt.legend(loc='upper left')
	plt.savefig(label[col-1]+'.png', bbox_inches='tight')
	#plt.show()
#plt.show()

	
