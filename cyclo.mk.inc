# add some self-inspection
CAT	= cat
MCSTRIP	= ./mcstrip
CYCLO	= ./cyclo

PRES	:= $(wildcard *.pre)
METS	:= $(wildcard *.mets)
STRPS	:= $(wildcard *.strp)
CYFLAGS	= -DANSI -I.
CYOUT	= $(METRICS)/raw_metrics_report.txt

.PHONEY: raw_metrics combine

rsrcs	= $(wildcard *.c)
METRICS	= cyclo-out

raw_metrics: $(rsrcs)

$(METRICS)/.keep_dir:
	mkdir -p $(dir $@)
	touch $@

$(rsrcs):
	@echo "$@" > $@.mets
	@$(CPP) $(CYFLAGS) $@ > $@.pre
	@$(MCSTRIP) $@.pre > $@.strp
	@$(CAT) $@.strp | $(CYCLO) -c >> $@.mets

combine: raw-metrics $(METRICS)/.keep_dir
	for file in $(METS) ; do \
		cat $$file >> $(CYOUT) ; \
		echo "" >> $(CYOUT) ; \
	done
	@echo "Metrics output now in $(CYOUT)"

