//! [ex_readAllUnreadCache]
std::vector<(void*)(TypeStructure*)> vec; //TypeStructure is your own define structure for the topic
readAllUnreadCache(&vec);
for (elements in vec)
	TypeStructure tp = *(TypeStructure*)vec[i];
	//Do something with tp.
//! [ex_readAllUnreadCache]

//! [ex_readMinSeqUnread]
TypeStructure tp; //TypeStructure is your own define structure for the topic
readMinSeqUnread((void*)&tp);
//! [ex_readMinSeqUnread]


