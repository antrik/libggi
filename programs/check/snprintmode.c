#include <ggi/gg-api.h>
#include <ggi/gii.h>
#include <ggi/ggi.h>

#include <strings.h>

int
main()
{
	int r, i;
	struct gg_stem *vis;
	ggi_mode mode;
	char buf[256];
	
	ggInit();
	ggiInit();
	
	vis = ggNewStem(libgii, libggi, NULL);
		
	ggiOpen(vis, NULL);

	bzero(&mode, sizeof(mode));
	bzero(&buf, sizeof(buf));
	ggiCheckMode(vis, &mode);

	for(i=0; i<50; i++) {
		r = ggiSNPrintMode(buf, i, &mode);
		printf("s=%i r=%i buf=\"%s\" (strlen=%i)\n",
		       i, r, buf, strlen(buf));
	}

	return 0;
}
