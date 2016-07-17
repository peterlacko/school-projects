<?php

/**
 * Homepage presenter.
 */
class HomepagePresenter extends BasePresenter
{
	private $fakultaRepository;
	private $zamereniRepository;

	protected function startup()
	{
	    parent::startup();
	    $this->fakultaRepository = $this->context->fakultaRepository;
		$this->zamereniRepository = $this->context->zamereniRepository;
	}

	public function renderDefault()
	{
	    $this->template->facultiesNames = $this->fakultaRepository->showFacultiesNames();
		$this->template->zamereni = $this->zamereniRepository->findAll();
	}

	public function deleteZamereni($nazev_zamereni) {
		$this->zamereniRepository->deleteZamereni($nazev_zamereni);
		$this->flashMessage('Zaměření bylo odstraněno', 'success');
	}


}
