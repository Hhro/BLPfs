FRONTENDSRC = src/frontend/
BACKENDSRC = src/backend/

DEPLOYDIR = deploy/
BACKENDDEPLOY = $(DEPLOYDIR)/backend/
PRODUCTIONDIR = production/
BACKENDPRODUCTION = $(PRODUCTIONDIR)/backend/

PRODUCTIONFLAG = THIS-IS-A-TEST-FLAG

build:
	$(MAKE) -C $(BACKENDSRC)

deploy: build
	mkdir -p $(DEPLOYDIR)

#Frontend
	#Copy frontend
	cp -r $(FRONTENDSRC) $(DEPLOYDIR)

#Backend
	mkdir -p $(DEPLOYDIR)/backend

	#Copy the file system manager
	cp $(BACKENDSRC)/build/blpfsm $(BACKENDDEPLOY)

	#Copy a fs generator
	cp $(BACKENDSRC)/tool/fs_gen $(BACKENDDEPLOY)

#Copy scripts & config
	cp scripts/cleaner.sh $(DEPLOYDIR)
	cp scripts/run.sh $(DEPLOYDIR)
	cp scripts/deploy.sh $(DEPLOYDIR)
	cp config/blpfsm.xinetd $(DEPLOYDIR)

production: build
	mkdir -p $(PRODUCTIONDIR)
#Frontend
	#Copy frontend
	cp -r $(FRONTENDSRC) $(PRODUCTIONDIR)

#Backend
	mkdir -p $(PRODUCTIONDIR)/backend

	#Copy the file system manager
	cp $(BACKENDSRC)/build/blpfsm $(BACKENDPRODUCTION)
	strip $(BACKENDPRODUCTION)/blpfsm

	#Copy a filesystem
	$(BACKENDSRC)/tool/fs_gen $(BACKENDPRODUCTION)/blpfs $(PRODUCTIONFLAG)
	
clean:
	#Clean src directory
	$(MAKE) clean -C $(BACKENDSRC)
	rm -rf $(PRODUCTIONDIR)
	rm -rf $(DEPLOYDIR)
