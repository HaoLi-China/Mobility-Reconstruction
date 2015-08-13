
#include "attribute_store.h"
#include "attribute_manager.h"




AttributeStore::~AttributeStore() {
	if(manager_ != nil) {
		manager_-> unregister_attribute_store(this) ;
	}
	manager_ = nil ;
}

void AttributeStore::bind(AttributeManager* manager) {
	ogf_assert(manager_ == nil) ;
	manager_ = manager ;
	if(manager_ != nil) {
		while(nb_chunks() < manager_->rat().nb_chunks()) {
			grow() ;
		}
		manager_-> register_attribute_store(this) ;
	}
}
