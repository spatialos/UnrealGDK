// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/OutgoingMessages.h"

namespace SpatialGDK
{
void FEntityQueryRequest::TraverseConstraint(Worker_Constraint* Constraint)
{
	switch (Constraint->constraint_type)
	{
	case WORKER_CONSTRAINT_TYPE_AND:
	{
		TUniquePtr<Worker_Constraint[]> NewConstraints =
			MakeUnique<Worker_Constraint[]>(Constraint->constraint.and_constraint.constraint_count);

		for (unsigned int i = 0; i < Constraint->constraint.and_constraint.constraint_count; i++)
		{
			NewConstraints[i] = Constraint->constraint.and_constraint.constraints[i];
			TraverseConstraint(&NewConstraints[i]);
		}
		Constraint->constraint.and_constraint.constraints = NewConstraints.Get();

		ConstraintStorage.Add(MoveTemp(NewConstraints));
		break;
	}
	case WORKER_CONSTRAINT_TYPE_OR:
	{
		TUniquePtr<Worker_Constraint[]> NewConstraints =
			MakeUnique<Worker_Constraint[]>(Constraint->constraint.or_constraint.constraint_count);

		for (unsigned int i = 0; i < Constraint->constraint.or_constraint.constraint_count; i++)
		{
			NewConstraints[i] = Constraint->constraint.or_constraint.constraints[i];
			TraverseConstraint(&NewConstraints[i]);
		}
		Constraint->constraint.or_constraint.constraints = NewConstraints.Get();

		ConstraintStorage.Add(MoveTemp(NewConstraints));
		break;
	}
	case WORKER_CONSTRAINT_TYPE_NOT:
	{
		TUniquePtr<Worker_Constraint[]> NewConstraint = MakeUnique<Worker_Constraint[]>(1);

		NewConstraint[0] = *Constraint->constraint.not_constraint.constraint;
		TraverseConstraint(&NewConstraint[0]);

		Constraint->constraint.not_constraint.constraint = NewConstraint.Get();

		ConstraintStorage.Add(MoveTemp(NewConstraint));
		break;
	}
	}
}

void SpatialMetrics::SendToConnection(Worker_Connection* Connection)
{
	// Do the conversion here so we can store everything on the stack.
	Worker_Metrics WorkerMetrics;

	WorkerMetrics.load = Load.IsSet() ? &Load.GetValue() : nullptr;

	TArray<Worker_GaugeMetric> WorkerGaugeMetrics;
	WorkerGaugeMetrics.SetNum(GaugeMetrics.Num());
	for (int i = 0; i < GaugeMetrics.Num(); i++)
	{
		WorkerGaugeMetrics[i].key = GaugeMetrics[i].Key.c_str();
		WorkerGaugeMetrics[i].value = GaugeMetrics[i].Value;
	}

	WorkerMetrics.gauge_metric_count = static_cast<uint32_t>(WorkerGaugeMetrics.Num());
	WorkerMetrics.gauge_metrics = WorkerGaugeMetrics.GetData();

	TArray<Worker_HistogramMetric> WorkerHistogramMetrics;
	TArray<TArray<Worker_HistogramMetricBucket>> WorkerHistogramMetricBuckets;
	WorkerHistogramMetrics.SetNum(HistogramMetrics.Num());
	WorkerHistogramMetricBuckets.SetNum(HistogramMetrics.Num());
	for (int i = 0; i < HistogramMetrics.Num(); i++)
	{
		WorkerHistogramMetrics[i].key = HistogramMetrics[i].Key.c_str();
		WorkerHistogramMetrics[i].sum = HistogramMetrics[i].Sum;

		WorkerHistogramMetricBuckets[i].SetNum(HistogramMetrics[i].Buckets.Num());
		for (int j = 0; j < HistogramMetrics[i].Buckets.Num(); j++)
		{
			WorkerHistogramMetricBuckets[i][j].upper_bound = HistogramMetrics[i].Buckets[j].UpperBound;
			WorkerHistogramMetricBuckets[i][j].samples = HistogramMetrics[i].Buckets[j].Samples;
		}

		WorkerHistogramMetrics[i].bucket_count = static_cast<uint32_t>(WorkerHistogramMetricBuckets[i].Num());
		WorkerHistogramMetrics[i].buckets = WorkerHistogramMetricBuckets[i].GetData();
	}

	WorkerMetrics.histogram_metric_count = static_cast<uint32_t>(WorkerHistogramMetrics.Num());
	WorkerMetrics.histogram_metrics = WorkerHistogramMetrics.GetData();

	Worker_Connection_SendMetrics(Connection, &WorkerMetrics);
}

} // namespace SpatialGDK
