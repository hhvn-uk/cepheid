#!/bin/awk -f

BEGIN {
	print "#include <check.h>"
	done = 0
	intest = 0
}

/^%{$/ {
	intest = 1
	print "START_TEST(ck_"test") {"
}

!/^%{$/ && (!intest || !/^}$/) {
	if (done)
		print "error: excess lines" > "/dev/stderr"
	else
		print
}

/^}$/ && intest {
	print "}"
	print "END_TEST"
	print ""
	print "#include <limits.h>"
	print "#include <unistd.h>"
	print "#include <stdlib.h>"
	print ""
	print "int"
	print "test_"test"(void) {"
	print "	Suite *s = suite_create(\""test"\");"
	print "	TCase *tc = tcase_create(\""test"\");"
	print "	SRunner *sr = srunner_create(s);"
	print "	char restoredir[PATH_MAX];"
	print "	char tmp[] = \"test.XXXXXX\";"
	print "	int ret;"
	print ""
	print "	getcwd(restoredir, sizeof(restoredir));"
	print "	mkdtemp(tmp);"
	print "	chdir(tmp);"
	print ""
	print "	suite_add_tcase(s, tc);"
	print "	tcase_add_test(tc, ck_"test");"
	print ""
	print "	srunner_run_all(sr, CK_ENV);"
	print "	ret = srunner_ntests_failed(sr);"
	print "	srunner_free(sr);"
	print ""
	print "	chdir(restoredir);"
	print "	/* rmdirp(tmp); */"
	print ""
	print "	return !!ret;"
	print "}"
	done = 1
}
