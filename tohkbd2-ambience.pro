TEMPLATE = aux

NAME=harbour-tohkbd2-ambience

OTHER_FILES = \
        harbour-tohkbd2.ambience \
        images/* \
        rpm/*

ambience.files = \
        harbour-tohkbd2.ambience
        
ambience.path = /usr/share/ambience/$${NAME}

images.files = images/*
images.path = $${ambience.path}/images

INSTALLS += \
        ambience \
        images

